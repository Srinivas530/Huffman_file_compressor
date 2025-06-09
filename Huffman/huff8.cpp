#include <iostream>
#include <fstream>
#include <unordered_map>
#include <queue>
#include <vector>
#include <bitset>
#include <chrono>
using namespace std;
using namespace chrono;

class Node {
public:
    char ch;
    int freq;
    Node *left, *right;

    Node(char c, int f) : ch(c), freq(f), left(nullptr), right(nullptr) {}
};

class Compare {
public:
    bool operator()(Node* a, Node* b) {
        return a->freq > b->freq;
    }
};

void buildCodes(Node* root, string code, unordered_map<char, string>& huffCodes) {
    if (!root) return;
    if (!root->left && !root->right) huffCodes[root->ch] = code;
    buildCodes(root->left, code + "0", huffCodes);
    buildCodes(root->right, code + "1", huffCodes);
}

Node* buildHuffmanTree(const unordered_map<char, int>& freqMap) {
    priority_queue<Node*, vector<Node*>, Compare> pq;
    for (auto& pair : freqMap)
        pq.push(new Node(pair.first, pair.second));
    while (pq.size() > 1) {
        Node* left = pq.top(); pq.pop();
        Node* right = pq.top(); pq.pop();
        Node* merged = new Node('\0', left->freq + right->freq);
        merged->left = left;
        merged->right = right;
        pq.push(merged);
    }
    return pq.top();
}

void writeBit(ofstream& out, string& buffer, char bit) {
    buffer += bit;
    if (buffer.size() == 8) {
        bitset<8> b(buffer);
        out.put((char)b.to_ulong());
        buffer.clear();
    }
}

void writeHeader(ofstream& out, unordered_map<char, string>& codes) {
    out.put((char)codes.size());
    for (auto& pair : codes) {
        out.put(pair.first);
        out.put((char)pair.second.length());
        // Write the code as bits packed into bytes:
        // We pack bits in bytes; since code length can be more than 8 bits, 
        // we write the string bits in a simplified way for now.
        // For simplicity, write code as string bits (0/1 chars).
        // You can optimize this later.
        for (char bit : pair.second) {
            out.put(bit);
        }
    }
}

unordered_map<string, char> readHeader(ifstream& in, uint32_t& totalBits) {
    unordered_map<string, char> decodeMap;
    int mapSize = in.get();
    for (int i = 0; i < mapSize; ++i) {
        char ch = in.get();
        int len = in.get();
        string code = "";
        for (int j = 0; j < len; ++j) {
            char bitChar = in.get();
            code += bitChar;
        }
        decodeMap[code] = ch;
    }
    // Read totalBits after header
    in.read(reinterpret_cast<char*>(&totalBits), sizeof(totalBits));
    return decodeMap;
}

void compress(const string& inputPath, const string& outputPath) {
    ifstream in(inputPath, ios::binary);
    ofstream out(outputPath, ios::binary);
    unordered_map<char, int> freqMap;
    char ch;
    string content;
    while (in.get(ch)) {
        freqMap[ch]++;
        content += ch;
    }
    in.close();

    auto t1 = high_resolution_clock::now();

    Node* root = buildHuffmanTree(freqMap);
    unordered_map<char, string> codes;
    buildCodes(root, "", codes);

    // Write header (with codes)
    // For each char: write char, length, then bits as chars '0'/'1'
    out.put((char)codes.size());
    for (auto& pair : codes) {
        out.put(pair.first);
        out.put((char)pair.second.length());
        for (char bit : pair.second)
            out.put(bit);
    }

    // Write body bits
    string buffer;
    uint32_t totalBits = 0;
    for (char c : content) {
        for (char bit : codes[c]) {
            buffer += bit;
            totalBits++;
            if (buffer.size() == 8) {
                bitset<8> b(buffer);
                out.put((char)b.to_ulong());
                buffer.clear();
            }
        }
    }
    // Padding last byte
    if (!buffer.empty()) {
        while (buffer.size() < 8) buffer += "0";
        bitset<8> b(buffer);
        out.put((char)b.to_ulong());
        buffer.clear();
    }

    // Write totalBits at the end of header section (before body actually)
    // But we wrote body already, so we move totalBits writing before body

    // So fix: write totalBits after header before body
    // To do this properly, let's close and reopen output for write header + totalBits then body.

    // To keep simple, here let's close and rewrite the file:
    out.close();

    // Reopen to rewrite header + totalBits correctly:
    ofstream out2(outputPath, ios::binary | ios::in | ios::out);
    // seek past header size (codes size + codes info)
    // But this needs a better design, so let's do a temp buffer approach instead in next iteration.

    // For now, to keep it simple and working, let's just write totalBits immediately after codes in the first write:
    // So better to rewrite compress function with that in mind (done below).

    auto t2 = high_resolution_clock::now();

    ifstream inSize(inputPath, ios::binary | ios::ate);
    ifstream outSize(outputPath, ios::binary | ios::ate);
    size_t origSize = inSize.tellg(), compSize = outSize.tellg();

    cout << "\n--- Compression Report ---\n";
    cout << "Original size     : " << origSize << " bytes\n";
    cout << "Compressed size   : " << compSize << " bytes\n";
    cout << "Compression ratio : " << ((100.0 * compSize) / origSize) << "%\n";
    cout << "Compression time  : " << duration_cast<milliseconds>(t2 - t1).count() << " ms\n";
}

// Rewritten compress for proper header and totalBits
void compress_fixed(const string& inputPath, const string& outputPath) {
    ifstream in(inputPath, ios::binary);
    unordered_map<char, int> freqMap;
    char ch;
    string content;
    while (in.get(ch)) {
        freqMap[ch]++;
        content += ch;
    }
    in.close();

    auto t1 = high_resolution_clock::now();

    Node* root = buildHuffmanTree(freqMap);
    unordered_map<char, string> codes;
    buildCodes(root, "", codes);

    ofstream out(outputPath, ios::binary);
    // Write number of codes
    out.put((char)codes.size());
    // Write codes map: char, length, bits as chars
    for (auto& pair : codes) {
        out.put(pair.first);
        out.put((char)pair.second.length());
        for (char bit : pair.second) {
            out.put(bit);
        }
    }

    // Calculate total bits to be written in body
    uint32_t totalBits = 0;
    for (char c : content)
        totalBits += codes[c].length();

    // Write totalBits (4 bytes)
    out.write(reinterpret_cast<char*>(&totalBits), sizeof(totalBits));

    // Write body bits
    string buffer;
    for (char c : content) {
        for (char bit : codes[c]) {
            buffer += bit;
            if (buffer.size() == 8) {
                bitset<8> b(buffer);
                out.put((char)b.to_ulong());
                buffer.clear();
            }
        }
    }
    if (!buffer.empty()) {
        while (buffer.size() < 8) buffer += "0";
        bitset<8> b(buffer);
        out.put((char)b.to_ulong());
    }

    out.close();

    auto t2 = high_resolution_clock::now();

    ifstream inSize(inputPath, ios::binary | ios::ate);
    ifstream outSize(outputPath, ios::binary | ios::ate);
    size_t origSize = inSize.tellg(), compSize = outSize.tellg();

    cout << "\n--- Compression Report ---\n";
    cout << "Original size     : " << origSize << " bytes\n";
    cout << "Compressed size   : " << compSize << " bytes\n";
    cout << "Compression ratio : " << ((100.0 * compSize) / origSize) << "%\n";
    cout << "Space saved " << ((100.0 * (origSize-compSize)) / origSize) << "%\n";
    cout << "Compression time  : " << duration_cast<milliseconds>(t2 - t1).count() << " ms\n";
}

void decompress(const string& inputPath, const string& outputPath) {
    ifstream in(inputPath, ios::binary);
    ofstream out(outputPath, ios::binary);

    auto t1 = high_resolution_clock::now();

    uint32_t totalBits;
    unordered_map<string, char> decodeMap = readHeader(in, totalBits);

    string bitBuffer, temp;
    char byte;

    while (in.get(byte)) {
        bitset<8> bits((unsigned char)byte);
        bitBuffer += bits.to_string();
    }

    // Use only totalBits number of bits, ignore padding bits
    string validBits = bitBuffer.substr(0, totalBits);

    for (char bit : validBits) {
        temp += bit;
        if (decodeMap.count(temp)) {
            out.put(decodeMap[temp]);
            temp = "";
        }
    }

    out.close();
    in.close();
    auto t2 = high_resolution_clock::now();

    ifstream outSize(outputPath, ios::binary | ios::ate);
    size_t decompressedSize = outSize.tellg();

    cout << "\n--- Decompression Report ---\n";
    cout << "Decompressed size : " << decompressedSize << " bytes\n";
    cout << "Decompression time: " << duration_cast<milliseconds>(t2 - t1).count() << " ms\n";
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        cerr << "Usage:\n";
        cerr << "  To compress:   " << argv[0] << " compress <input.txt> <compressed.huff>\n";
        cerr << "  To decompress: " << argv[0] << " decompress <compressed.huff> <output.txt>\n";
        return 1;
    }

    string mode = argv[1];
    string inputFile = argv[2];
    string outputFile = argv[3];

    if (mode == "compress") {
        compress_fixed(inputFile, outputFile);
    } else if (mode == "decompress") {
        decompress(inputFile, outputFile);
    } else {
        cerr << "Unknown mode: " << mode << "\n";
        return 1;
    }

    return 0;
}

