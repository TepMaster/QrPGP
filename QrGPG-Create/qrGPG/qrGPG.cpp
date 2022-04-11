#include <climits>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "qrcodegen.hpp"
#include<fstream>
#include "sha256.h"
#include "json.hpp"
#include"base64.h"
#define MAXSIZE 1800
//Qr namespace
using std::uint8_t;
using qrcodegen::QrCode;
using qrcodegen::QrSegment;
using json = nlohmann::json;

static std::string toSvgString(const QrCode& qr, int border);
static std::string toSvgString(const QrCode& qr, int border) {
	if (border < 0)
		throw std::domain_error("Border must be non-negative");
	if (border > INT_MAX / 2 || border * 2 > INT_MAX - qr.getSize())
		throw std::overflow_error("Border too large");

	std::ostringstream sb;
	sb << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	sb << "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n";
	sb << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" viewBox=\"0 0 ";
	sb << (qr.getSize() + border * 2) << " " << (qr.getSize() + border * 2) << "\" stroke=\"none\">\n";
	sb << "\t<rect width=\"100%\" height=\"100%\" fill=\"#FFFFFF\"/>\n";
	sb << "\t<path d=\"";
	for (int y = 0; y < qr.getSize(); y++) {
		for (int x = 0; x < qr.getSize(); x++) {
			if (qr.getModule(x, y)) {
				if (x != 0 || y != 0)
					sb << " ";
				sb << "M" << (x + border) << "," << (y + border) << "h1v1h-1z";
			}
		}
	}
	sb << "\" fill=\"#000000\"/>\n";
	sb << "</svg>\n";
	return sb.str();
}

int i = 1;

void save(std::string data, int in,std::string name) {
	FILE* fp;
	std::string path = name+"." ;
	path = path + std::to_string(in);
	path.append(".svg");

	fopen_s(&fp, path.c_str(), "w");
	fprintf(fp, data.c_str());

	fclose(fp);

}

std::string load(std::string name) {

	std::ifstream inFile;
	inFile.open(name);

	std::stringstream strStream;
	strStream << inFile.rdbuf(); //read the file
	std::string str = strStream.str(); //str holds the content of the file

	return str;
}



//main
int main(int argc, char* argv[]) {
	
	//arg handeling
	if (argc < 2) {
	//mesaj eroare
		std::cout << "Usage: filename to key converted"<< std::endl;

		return 1;
	}
	//2953 
	int curent=0;
	//qr setup
	const QrCode::Ecc errCorLvl = QrCode::Ecc::MEDIUM;  // Error correction level

	std::string data = load(argv[1]);
	std::string path = argv[1];
	std::string name = path.substr(path.find_last_of("/\\") + 1);
	std::cout << "Nr de bytes: " << data.length() << std::endl;
	while (data.length() > curent) {
		if (data.length() > MAXSIZE) {
			std::cout << i<<"/"<<(data.length() / MAXSIZE) + 1 << "parti"<<"\n";
			std::string part = data.substr(curent, MAXSIZE);
			json j;
			j["nr"] = i;
			j["fname"] = name;
			j["nrtot"] = (data.length() / MAXSIZE)+1;
			j["data"] = base64_encode(part);
			j["hash"] = sha256(base64_encode(part));
			const QrCode qr = QrCode::encodeText(j.dump().c_str(), errCorLvl);
			save(toSvgString(qr, 8), i, name);
			i++;
		}

		if(MAXSIZE> data.length()) {
			//json encodare
			std::cout << "Doar o parte";
			json j;
			j["nr"] = 1;
			j["fname"] = name;
			j["nrtot"] = data.length() / MAXSIZE;
			j["data"] = base64_encode(data);
			j["hash"] = sha256(base64_encode(data));

			const QrCode qr = QrCode::encodeText(j.dump().c_str(), errCorLvl);
			save(toSvgString(qr, 8), i, name);

		}
		curent = curent + MAXSIZE;
	}
	

	
	system("pause");
	return 0;
}

