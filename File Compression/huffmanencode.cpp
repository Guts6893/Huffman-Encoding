#include <bits/stdc++.h>
using namespace std;

// Node of the Huffman Tree
class TreeNode
{
public:
    char data;
    int freq;
    TreeNode *left, *right;
    TreeNode(char d, int f)
    {
        data = d;
        freq = f;
        left = nullptr;
        right = nullptr;
    }
};

// For min-heap based on frequency
class Compare
{
public:
    bool operator()(TreeNode *a, TreeNode *b)
    {
        return a->freq > b->freq;
    }
};

class Huffman
{
public:
    unordered_map<char, string> encodeMap;
    unordered_map<string, char> decodeMap;

    void generateCodes(TreeNode *node, string code);
    void Encode(const string &inputFileName, const string &outputFileName);
    void Decode(const string &inputFileName, const string &outputFileName);
};

void Huffman::generateCodes(TreeNode *node, string code)
{
    if (!node) return;
    if (!node->left && !node->right)
    {
        encodeMap[node->data] = code;
        decodeMap[code] = node->data;
    }

    generateCodes(node->left, code + "0");
    generateCodes(node->right, code + "1");
}

void Huffman::Encode(const string &inputFileName, const string &outputFileName)
{
    ifstream input(inputFileName, ios::binary);
    ofstream output(outputFileName, ios::binary);

    if (!input.is_open() || !output.is_open())
    {
        cerr << "Error: Unable to open files.\n";
        return;
    }

    input.seekg(0, ios::end);
    int originalSize = input.tellg();
    input.seekg(0, ios::beg);

    unordered_map<char, int> frequency;
    char ch;
    while (input.get(ch))
        frequency[ch]++;

    input.clear();
    input.seekg(0);

    priority_queue<TreeNode *, vector<TreeNode *>, Compare> pq;
    for (auto &entry : frequency)
        pq.push(new TreeNode(entry.first, entry.second));

    while (pq.size() > 1)
    {
        TreeNode *left = pq.top(); pq.pop();
        TreeNode *right = pq.top(); pq.pop();
        TreeNode *parent = new TreeNode('\0', left->freq + right->freq);
        parent->left = left;
        parent->right = right;
        pq.push(parent);
    }

    TreeNode *huffmanRoot = pq.top();
    generateCodes(huffmanRoot, "");

    int mapSize = frequency.size();
    output.write((char *)&mapSize, sizeof(int));
    for (auto &entry : encodeMap)
    {
        output.write((char *)&entry.first, sizeof(char));
        int codeLength = entry.second.size();
        output.write((char *)&codeLength, sizeof(int));
        output.write(entry.second.c_str(), codeLength);
    }

    string encodedString = "";
    while (input.get(ch))
        encodedString += encodeMap[ch];

    int extraBits = (8 - (encodedString.size() % 8)) % 8;
    encodedString.append(extraBits, '0');

    output.write((char *)&extraBits, sizeof(int));
    for (size_t i = 0; i < encodedString.size(); i += 8)
    {
        string byte = encodedString.substr(i, 8);
        char byteValue = stoi(byte, nullptr, 2);
        output.write(&byteValue, sizeof(char));
    }

    output.close();
    input.close();

    ifstream compressedFile(outputFileName, ios::binary);
    compressedFile.seekg(0, ios::end);
    int compressedSize = compressedFile.tellg();
    compressedFile.close();

    double compressionRatio = static_cast<double>(compressedSize) / originalSize;
    double compressionPercent = (1.0 - compressionRatio) * 100;

    cout << "Compression Successful\n";
    cout << "Original Size: " << originalSize << " bytes\n";
    cout << "Compressed Size: " << compressedSize << " bytes\n";
    cout << "Compression Ratio: " << compressionRatio << "\n";
    cout << "Compression Percentage: " << compressionPercent << "%\n";
}

void Huffman::Decode(const string &inputFileName, const string &outputFileName)
{
    ifstream input(inputFileName, ios::binary);
    ofstream output(outputFileName, ios::binary);

    if (!input.is_open() || !output.is_open())
    {
        cerr << "Error: Unable to open files.\n";
        return;
    }

    int uniqueCharCount;
    input.read((char *)&uniqueCharCount, sizeof(int));
    decodeMap.clear();

    for (int i = 0; i < uniqueCharCount; ++i)
    {
        char character;
        int codeLength;
        input.read(&character, sizeof(char));
        input.read((char *)&codeLength, sizeof(int));
        char *code = new char[codeLength + 1];
        input.read(code, codeLength);
        code[codeLength] = '\0';
        decodeMap[string(code)] = character;
        delete[] code;
    }

    int extraBits;
    input.read((char *)&extraBits, sizeof(int));
    string binaryData = "";
    char byte;
    while (input.read(&byte, sizeof(char)))
        binaryData += bitset<8>(static_cast<unsigned char>(byte)).to_string();

    binaryData = binaryData.substr(0, binaryData.size() - extraBits);

    string currentCode = "";
    for (char bit : binaryData)
    {
        currentCode += bit;
        if (decodeMap.count(currentCode))
        {
            output.put(decodeMap[currentCode]);
            currentCode.clear();
        }
    }

    input.close();
    output.close();
    cout << "Decompression Successful\n";
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        cerr << "Usage: " << argv[0] << " <input_file> <output_file> <compress/decompress>\n";
        return 1;
    }

    string inputFile = argv[1];
    string outputFile = argv[2];
    string mode = argv[3];

    Huffman huffman;

    if (mode == "compress")
        huffman.Encode(inputFile, outputFile);
    else if (mode == "decompress")
        huffman.Decode(inputFile, outputFile);
    else
    {
        cerr << "Invalid mode. Use 'compress' or 'decompress'.\n";
        return 1;
    }

    return 0;
}
