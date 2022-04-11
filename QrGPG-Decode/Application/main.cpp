#include <opencv2/highgui/highgui.hpp>  
#include <opencv2/imgproc/imgproc.hpp>  
#include <zbar.h>  
#include <iostream>  
#include"base64.h"
#include"sha256.h"
#include"json.hpp"
#include <Windows.h>
#include <string> 
//functi
void HideConsole()
{
    ::ShowWindow(::GetConsoleWindow(), SW_HIDE);
}
void clear() {
    // CSI[2J clears screen, CSI[H moves the cursor to top-left corner
    std::cout << "\x1B[2J\x1B[H";
}
void ShowConsole()
{
    ::ShowWindow(::GetConsoleWindow(), SW_SHOW);
}
void save(std::string data, std::string name) {
    FILE* fp;

    fopen_s(&fp, name.c_str(), "w");
    fprintf(fp, data.c_str());
    fclose(fp);

}
//namespace
using json = nlohmann::json;
using namespace cv;
using namespace std;
using namespace zbar;
//linux comp
//g++ main.cpp /usr/local/include/ /usr/local/lib/ -lopencv_highgui.2.4.8 -lopencv_core.2.4.8  
int main(int argc, char* argv[])
{
    HideConsole();//hid la cmd
    bool first = true;
    bool running = true;
    std::string nexdata="";
    int curent = 1;
    std::string dat;
    
    VideoCapture cap(0); // set ce camera sa fol
    //size overwrite
    // cap.set(CV_CAP_PROP_FRAME_WIDTH,800);  
    // cap.set(CV_CAP_PROP_FRAME_HEIGHT,640);  
    if (!cap.isOpened()) // if not success, exit program  
    {
        cout << "Cannot open the video cam" << endl;
        return -1;
    }
    //opecl ini
    ImageScanner scanner;
    scanner.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 1);
    double dWidth = cap.get(3); //get the width of frames of the video  
    double dHeight = cap.get(4); //get the height of frames of the video  
    cout << "Frame size : " << dWidth << " x " << dHeight << endl;
    namedWindow("QrGPG_Decoder", WINDOW_AUTOSIZE); //create a window 
    //main loop
    while (running)
    {
        Mat frame;
        bool suc = cap.read(frame); // read a new frame from camera  
        if (!suc) //daca ii eroare se opreste
        {
            cout << "Webcam error" << endl;//end 
            break;
        }

        Mat grey;//B\W filtru
        cvtColor(frame, grey, COLOR_BGR2GRAY);
        int width = frame.cols;
        int height = frame.rows;
        uchar* raw = (uchar*)grey.data;
        // wrap image data   
        Image image(width, height, "Y800", raw, width * height);
        // scan the image for barcodes   
        int n = scanner.scan(image);
        //status-gui
        if (nexdata != "0") {
            putText(frame, //target image
                nexdata, //text
                cv::Point(10, frame.rows / 2), //top-left position
                cv::FONT_HERSHEY_DUPLEX,
                1.0,
                CV_RGB(180, 5, 5), //font color
                2);
        }
        //next
        if (first == false) {
            putText(frame, //target image
                "Next Qr Code", //text
                cv::Point(frame.cols/2, frame.rows -30), //top-left position
                cv::FONT_HERSHEY_DUPLEX,
                1.0,
                CV_RGB(200, 5, 3), //font color
                2);
        }


        //Start message::
        if (first == true) {
            putText(frame, //target image
                "Show first Qr Code", //text
                cv::Point(frame.cols / 4, 30), //top-left position
                cv::FONT_HERSHEY_DUPLEX,
                1.0,
                CV_RGB(50, 203, 201), //font color
                2);
        }
      

        // extract results   
        for (Image::SymbolIterator symbol = image.symbol_begin();
            symbol != image.symbol_end();
            ++symbol) {
            vector<Point> vp;
            //qr data handle

            //daca ii tip QR
            if (symbol->get_type_name() == "QR-Code") {
               
               
                auto j = json::parse(symbol->get_data());//dau parse la json
                //verific integritatea
                if (sha256(j["data"].get<std::string>()) == j["hash"].get<std::string>()) {
                    first = false;//setez ca nu mai e primul
                    cout << "Qr decoded ok!" << endl;
                    std::string fname = j["fname"].get<std::string>();
                    //daca este doar compus din 1
                    if (j["nrtot"].get<int>() == 0) {
                       
                        clear();
                        ShowConsole();
                        std::cout << base64_decode(j["data"].get<std::string>())<<endl;
                        save(base64_decode(j["data"].get<std::string>()), fname);
                        std::cout << "Saved under:" << fname << endl;
                        system("pause");
                        running = false;
                        break;
                    }
                    //daca sunt mai multe qr

                    cout << j["nr"].get<int>() << "/" << j["nrtot"].get<int>();
                    nexdata = std::to_string(j["nr"].get<int>())+ "/" + std::to_string(j["nrtot"].get<int>());
                    if (j["nrtot"].get<int>() >= j["nr"].get<int>()) {
                        if (j["nr"].get<int>()== curent) {
                            cout << curent << endl << j["nrtot"].get<int>()<<endl;
                            dat = dat + base64_decode(j["data"].get<std::string>());
                            curent++;
                        }
                        if (j["nr"].get<int>() == j["nrtot"].get<int>()) {
                            clear();
                            ShowConsole();
                            std::cout << dat << endl;
                            save(dat, fname);
                            std::cout << "Saved under:" << fname<<endl;
                            system("pause");
                            running = false;
                            break;
                        }
                    }

                    
                }




              
            }

                int n = symbol->get_location_size();
            for (int i = 0; i < n; i++) {
                vp.push_back(Point(symbol->get_location_x(i), symbol->get_location_y(i)));
            }
            RotatedRect r = minAreaRect(vp);
            Point2f pts[4];
            r.points(pts);
            for (int i = 0; i < 4; i++) {
                line(frame, pts[i], pts[(i + 1) % 4], Scalar(255, 0, 0), 3);
            }  
        }
        imshow("QrGPG_Decoder", frame); 
        if (waitKey(30) == 27) //esp escape
        {
            cout << "Program Closed" << endl;
            break;
        }
    } 
    return 0;
}