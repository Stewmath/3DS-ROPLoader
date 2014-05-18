#include <fstream>

using namespace std;

const int rows = 0x40;

int main(int argc, char* argv[]) {
    ifstream in("../source/data", ios::binary);
    char data[rows*16];
    in.read(data, rows*16);
    ofstream out("../source/data.h");
    
    out << "u8 data[] = {\n";

    for (int i=0; i<rows; i++) {
        for (int j=0; j<16; j++) {
            out << hex << "0x" << (int)((unsigned char)data[i*16+j]) << ", ";
        }
        out << "\n";
    }
    out << "0};";

    in.close();
    out.close();
}
