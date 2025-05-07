#include<bits/stdc++.h>
using namespace std;

#define io                        \
    ios_base::sync_with_stdio(0); \
    cin.tie(0);                   \
    cout.tie(0);

unordered_map<char, string> hashmap;

struct huffnode
{
    char character;
    unsigned int freq;
    struct huffnode *left, *right;
};
typedef struct huffnode *NODE;

struct Minheap
{
    unsigned short size, capacity;
    NODE *array;
};
typedef struct Minheap *HEAP;

NODE newleafnode(char c, int f)
{
    NODE tmp = new huffnode();
    tmp->character = c;
    tmp->freq = f;
    tmp->left = NULL;
    tmp->right = NULL;
    return tmp;
}

NODE newinternalnode(int f)
{
    NODE tmp = new huffnode();
    tmp->character = 0;
    tmp->freq = f;
    tmp->left = NULL;
    tmp->right = NULL;
    return tmp;
}

void swap(NODE *a, int i, int j)
{
    NODE tmp = a[i];
    a[i] = a[j];
    a[j] = tmp;
}

void heapify(HEAP heap)
{
    int i, j, k;
    unsigned short n = heap->size;
    NODE *a = heap->array;
    for (i = n / 2; i > 0; i--)
    {
        if (2 * i + 1 <= n)
            j = a[2 * i]->freq < a[2 * i + 1]->freq ? 2 * i : 2 * i + 1;
        else
            j = 2 * i;

        if (a[j]->freq < a[i]->freq)
        {
            swap(a, i, j);
            while (j <= n / 2 && (a[j]->freq > a[2 * j]->freq || (2 * j + 1 <= n ? a[j]->freq > a[2 * j + 1]->freq : 0)))
            {
                if (2 * j + 1 <= n)
                    k = a[2 * j]->freq < a[2 * j + 1]->freq ? 2 * j : 2 * j + 1;
                else
                    k = 2 * j;
                swap(a, k, j);
                j = k;
            }
        }
    }
}

NODE mintop(HEAP heap)
{
    NODE top = heap->array[1];
    heap->array[1] = heap->array[heap->size--];
    heapify(heap);
    return top;
}

void insertnode(HEAP heap, NODE leftchild, NODE rightchild, int f)
{
    NODE tmp = new huffnode();
    tmp->character = '\0';
    tmp->freq = f;
    tmp->left = leftchild;
    tmp->right = rightchild;
    heap->array[++heap->size] = tmp;
    heapify(heap);
}

void huffman_tree(HEAP heap)
{
    while (heap->size > 1)
    {
        NODE left_child = mintop(heap);
        NODE right_child = mintop(heap);
        insertnode(heap, left_child, right_child, (left_child->freq + right_child->freq));
    }
}

void assign_codes(NODE root, string str)
{
    if (!root->left && !root->right)
    {
        hashmap[root->character] = str;
    }
    else
    {
        assign_codes(root->left, str + "0");
        assign_codes(root->right, str + "1");
    }
}

int main(int argc, char *argv[])
{
    io;
    if (argc != 2)
    {
        cout << "Invalid cmd line arg. Usage: ./a.out <input file>\n";
        return 1;
    }
    
    FILE *input = fopen(argv[1], "r");
    if (input == NULL)
    {
        cout << "Error opening input file\n";
        return 2;
    }

    string in = argv[1];
    string filetype = in.substr(in.find(".") + 1);
    cout << filetype << "\n";
    string out = in.substr(0, in.find(".")) + "-compressed.bin";

    FILE *output = fopen(out.c_str(), "wb");
    if (output == NULL)
    {
        cout << "Error creating output file\n";
        return 3;
    }

    // Write file type
    int ftsize = filetype.length();
    char num = ftsize + '0';
    fwrite(&num, 1, 1, output);
    for (int i = 0; i < ftsize; i++)
    {
        char buff = filetype[i];
        fwrite(&buff, 1, 1, output);
    }

    // Holds frequency of each unique character
    int freq[256] = {0};
    unsigned short num_of_unique_chars = 0;
    char buff[1];
    while (fread(buff, 1, 1, input))
    {
        if (!freq[buff[0]])
            num_of_unique_chars++;
        freq[buff[0]]++;
    }
    fclose(input);

    FILE *input1 = fopen(argv[1], "r");

    HEAP heap = new Minheap();
    heap->capacity = num_of_unique_chars + 1;
    heap->size = 0;
    heap->array = new NODE[heap->capacity];

    for (int i = 0; i < 256; i++)
    {
        if (freq[i])
        {
            heap->size += 1;
            heap->array[heap->size] = newleafnode(i, freq[i]);
        }
    }

    heapify(heap);

    huffman_tree(heap);
    assign_codes(heap->array[heap->size], "");

    string file_contents = "";
    while (fread(buff, 1, 1, input1))
    {
        file_contents += hashmap[buff[0]];
    }

    // Find padding to make binary code length a multiple of 8
    int padding = 0;
    if (file_contents.length() % 8 != 0)
        padding = 8 - (file_contents.length() % 8);
    for (int i = 0; i < padding; i++)
    {
        file_contents = "0" + file_contents;
    }

    // Write the codes assigned to characters at the start of the file
    string codes;
    for (auto &iter : hashmap)
    {
        codes = iter.first + iter.second;
        fwrite(codes.c_str(), codes.length() + 1, 1, output);
    }

    // Write padding
    char null = '\0';
    char pad = padding + '0';
    fwrite(&null, 1, 1, output);
    fwrite(&pad, 1, 1, output);
    fwrite(&null, 1, 1, output);
    fwrite(&null, 1, 1, output);

    // Convert binary code to decimal and write to the output file
    for (int j = 0; file_contents[j]; j += 8)
    {
        char tmp_bin[9];
        strncpy(tmp_bin, &file_contents[j], 8);
        int dec = 0;
        for (int i = 0; i < 8; i++)
        {
            dec = (dec << 1) | (tmp_bin[i] == '1' ? 1 : 0);
        }
        fwrite(&dec, 1, 1, output);
    }

    cout << "Compressed File successfully\n";

    fclose(input1);
    fclose(output);

    delete[] heap->array;
    delete heap;

    return 0;
}
