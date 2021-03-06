#include "ofxBDF.h"

void ofxBDF::setup(string path) {
	ifstream inputFile;
	inputFile.open(path);
	if (!inputFile) {
		error(1, "ofxBDF ERROR - Unable to load! File from " + path);
	}
	parseInputFile(inputFile);
	inputFile.close();
}

void ofxBDF::draw(int scale) {
	int x = metadata.size;
	int y = metadata.size;
	for (int i = 0; i < chars.size(); i++) {
		chars[i].draw(x, y, scale);
		x += chars[i].character.getWidth() * scale;
		if (x > ofGetWidth() - metadata.size * 2) {
			x = metadata.size;
			y += metadata.mainBoundingBox.height * scale;
		}
	}
}

ofxBDFMetadata ofxBDF::getMetadata() const {
	return metadata;
}

void ofxBDF::parseInputFile(ifstream &file) {

	string line;
	stack<Declaration> declarations;
	ofxBDFChar currentChar;

	while (getline(file, line)) {
		vector<string> tokens = getTokens(line);
		Declaration currentDeclaration = toDeclaration(tokens[0]);

		switch (currentDeclaration) {
		case STARTFONT:
			declarations.push(currentDeclaration);
			metadata.versionNumber = tokens[1];
			break;
		case FONT:
			metadata.name = tokens[1];
			break;
		case SIZE:
			metadata.size = stoi(tokens[1]);
			metadata.xDPI = stoi(tokens[2]);
			metadata.yDPI = stoi(tokens[3]);
			break;
		case FONTBOUNDINGBOX:
			metadata.mainBoundingBox.width = stoi(tokens[1]);
			metadata.mainBoundingBox.height = stoi(tokens[2]);
			metadata.mainBoundingBox.x = stoi(tokens[3]);
			metadata.mainBoundingBox.y = stoi(tokens[4]);
			break;
		case STARTPROPERTIES:
			declarations.push(currentDeclaration);
			break;
		case FONT_DESCENT:
			metadata.fontDescent = stoi(tokens[1]);
			break;
		case FONT_ASCENT:
			metadata.fontAssent = stoi(tokens[1]);
			break;
		case DEFAULT_CHAR:
			metadata.defaultChar = stoi(tokens[1]);
			break;
		case ENDPROPERTIES:
			declarations.pop();
			break;
		case CHARS:
			metadata.numberOfChars = stoi(tokens[1]); // TODO add check
			break;
		case STARTCHAR:
			declarations.push(currentDeclaration);
			currentChar.name = tokens[1];
			break;
		case ENCODING:
			currentChar.code = stoi(tokens[1]);
			break;
		case SWIDTH:
			currentChar.scalableWidthX = stoi(tokens[1]);
			currentChar.scalableWidthY = stoi(tokens[2]);
			break;
		case DWIDTH:
			currentChar.deviceWidthX = stoi(tokens[1]);
			currentChar.deviceWidthY = stoi(tokens[2]);
			break;
		case BBX:
			ofxBDFBoundingBox box;
			box.width = stoi(tokens[1]);
			box.height = stoi(tokens[2]);
			box.x = stoi(tokens[3]);
			box.y = stoi(tokens[4]);
			if (metadata.mainBoundingBox == box) {
				currentChar.boundingBox = metadata.mainBoundingBox;
			}
			else {
				currentChar.boundingBox = box;
			}
			break;
		case BITMAP:
			currentChar.character.allocate(currentChar.boundingBox.width, currentChar.boundingBox.height, OF_IMAGE_COLOR_ALPHA);
			for (int i = 0; i < currentChar.boundingBox.height; i++) {
				getline(file, line);
				unsigned long long rawLine = strtoll(line.c_str(), 0, 16);
				int bitsToUse = sizeof(unsigned long long) * 8;
				if (currentChar.boundingBox.width < bitsToUse) {
					bitsToUse = currentChar.boundingBox.width;
				}
				int padding = currentChar.boundingBox.width % 8 == 0 ? 0 : 8 - (currentChar.boundingBox.width % 8);
				for (int j = 0; j < bitsToUse; j++) {
					int x = ((bitsToUse - 1) - j);
					if (x < currentChar.character.getWidth()) {
						if (rawLine >> (j + padding) & 0x1) {
							currentChar.character.setColor(x, i, ofColor::black);
						}
						else {
							currentChar.character.setColor(x, i, ofColor(255, 255, 255, 0));
						}
					}
				}
			}
			break;
		case ENDCHAR:
			declarations.pop();
			chars.push_back(currentChar);
			currentChar = ofxBDFChar();
			break;
		case ENDFONT:
			declarations.pop();
			break;
		case UNKNOWN:
			warn("Unexpected line when parsing file! Occurred on line : \"" + line + "\"");
			break;
		}
	}

	if (!declarations.empty()) {
		error(2, "Declaration in file was not closed!");
	}

}

vector<string> ofxBDF::getTokens(string s) {
	string buffer;
	stringstream stream(s);
	vector<string> tokens;
	while (stream >> buffer) {
		tokens.push_back(buffer);
	}
	return tokens;
}

void ofxBDF::error(int code, string description) {
	cout << "ofxBDF ERROR - ";
	cout << description << endl;
	cout << "Press enter to exit.";
	cin.get();
	exit(code);
}

void ofxBDF::warn(string description) {
	cout << "ofxBDF WARN - ";
	cout << description << endl;
}

ofxBDF::Declaration ofxBDF::toDeclaration(string const &s) {
	if (s == "STARTFONT"      ) return STARTFONT;
	if (s == "FONT"           ) return FONT;
	if (s == "SIZE"           ) return SIZE;
	if (s == "FONTBOUNDINGBOX") return FONTBOUNDINGBOX;
	if (s == "STARTPROPERTIES") return STARTPROPERTIES;
	if (s == "FONT_DESCENT"   ) return FONT_DESCENT;
	if (s == "FONT_ASCENT"    ) return FONT_ASCENT;
	if (s == "DEFAULT_CHAR"   ) return DEFAULT_CHAR;
	if (s == "ENDPROPERTIES"  ) return ENDPROPERTIES;
	if (s == "CHARS"          ) return CHARS;
	if (s == "STARTCHAR"      ) return STARTCHAR;
	if (s == "ENCODING"       ) return ENCODING;
	if (s == "SWIDTH"         ) return SWIDTH;
	if (s == "DWIDTH"         ) return DWIDTH;
	if (s == "BBX"            ) return BBX;
	if (s == "BITMAP"         ) return BITMAP;
	if (s == "ENDCHAR"        ) return ENDCHAR;
	if (s == "ENDFONT"        ) return ENDFONT;
	return UNKNOWN;
}
