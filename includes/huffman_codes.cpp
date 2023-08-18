#ifndef HUFFMAN_CODES_CPP
#define HUFFMAN_CODES_CPP

#include <bits/stdc++.h>
using namespace std;

#define NOT_ASCII_CHAR '['

struct MinHeapNode{
    char d;
    unsigned long long frequency;
    MinHeapNode *lChild, *rChild;

    MinHeapNode(char d, unsigned long long frequency){

        lChild = NULL;
        rChild = NULL;
        this->d = d;
        this->frequency = frequency;
    }
};

//function to compare
struct compare {
    bool operator()(MinHeapNode *l, MinHeapNode *r){
        return (l->frequency > r->frequency);
    }
};

void make_encoder(struct MinHeapNode *root, std::vector<string> & encoder, string str = ""){
    if (!root)
        return;

    if (root->d != NOT_ASCII_CHAR)
        encoder[root->d] = str;

    make_encoder(root->lChild, encoder, str + "0");
    make_encoder(root->rChild, encoder, str + "1");
}

void huffman_codes(std::map<char, long long unsigned> global_counts, std::vector<string> & encoder, MinHeapNode* & decoder){

    struct MinHeapNode *lChild, *rChild, *top;

    priority_queue<MinHeapNode *, vector<MinHeapNode *>, compare> minHeap;

    for (auto value : global_counts){
        minHeap.push(new MinHeapNode(value.first, value.second));
    }
    
    while (minHeap.size() != 1){

        lChild = minHeap.top();
        minHeap.pop();

        rChild = minHeap.top();
        minHeap.pop();

        top = new MinHeapNode(NOT_ASCII_CHAR, lChild->frequency + rChild->frequency);

        top->lChild = lChild;
        top->rChild = rChild;

        minHeap.push(top);
    }

    make_encoder(minHeap.top(), encoder);

    decoder = minHeap.top();
}

void decode(MinHeapNode* root, string s, long long unsigned &index, string &decoded){
    if (root == NULL)
        return;

    if (root->d != NOT_ASCII_CHAR){
        decoded += root->d;
        return;
    }

    index++;

    if (s[index] == '0')
        decode(root->lChild, s, index, decoded);
    else
        decode(root->rChild, s, index, decoded);
}

void clean_up(MinHeapNode* & root){
    if (root == NULL)
        return;

    clean_up(root->lChild);
    clean_up(root->rChild);

    delete root;
}


#endif


