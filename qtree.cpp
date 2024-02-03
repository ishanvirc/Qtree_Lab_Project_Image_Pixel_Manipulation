/**
 * @file qtree.cpp
 */

#include "qtree.h"


QTree::QTree(const PNG& imIn) {
    height = imIn.height();
    width = imIn.width(); //     ul                  lr 
    root = BuildNode(imIn, make_pair(0, 0), make_pair(width - 1, height -1));
}

QTree& QTree::operator=(const QTree& rhs) {
        if (this != &rhs) {
        Clear();
        height = rhs.height;
        width = rhs.width;
        Copy(rhs);
    }
    return *this;
}

PNG QTree::Render(unsigned int scale) const {
    PNG result(width * scale, height * scale);
    renderHelper(root, &result, scale);
    return result;
}


void QTree::Prune(double tolerance) {
    pruneHelper(root, tolerance);
}

void QTree::FlipHorizontal() {
    flipHorizontalHelper(root, width - 1);
}

void QTree::RotateCCW() {
    rotateCCWHelper(root);
    cout << "Width, Height Before Swap: " << width << ", " << height << endl;
    swap(width, height);
    cout << "Width, Height After Swap: " << width << ", " << height << endl;
    flipVerticalHelper(root, height - 1);
}

void QTree:: Clear() {
    clearHelper(root);
    root = nullptr;
}

void QTree::Copy(const QTree& other) {
    root = copyHelper(other.root);
}

Node* QTree::BuildNode(const PNG& img, pair<unsigned int, unsigned int> ul, pair<unsigned int, unsigned int> lr) {
    // (single pixel node)
    if (ul.first == lr.first && ul.second == lr.second) {
        RGBAPixel* pixel = img.getPixel(ul.first, ul.second);
        return new Node(ul, lr, *pixel);
    } 
   
    // case 1 (width is exactly 1)
    else if (ul.first == lr.first) {
        unsigned int midY = static_cast<unsigned int>((ul.second + lr.second) / 2);
        //                              dummy 
        Node* node = new Node(ul, lr, RGBAPixel());
        //                        ul                lr
        node->NW = BuildNode(img, ul, make_pair(ul.first, midY));  
        node->SW = BuildNode(img, make_pair(ul.first, midY + 1), lr);
        node->avg = calculateAverageColor(node);
        return node;
    } 
    
    // case 2 (height is exactly 1)
    else if (ul.second == lr.second) {
        unsigned int midX = static_cast<unsigned int>((ul.first + lr.first) / 2);
        //                              dummy 
        Node* node = new Node(ul, lr, RGBAPixel());
        //                        ul                lr
        node->NW = BuildNode(img, ul, make_pair(midX, ul.second));
        node->NE = BuildNode(img, make_pair(midX + 1, ul.second), lr);
        node->avg = calculateAverageColor(node);
        return node;
    }
    
    // case 3, 4, 5 
    else {
        unsigned int midX = static_cast<unsigned int>((ul.first + lr.first) / 2);
        unsigned int midY = static_cast<unsigned int>((ul.second + lr.second) / 2);
        Node* node = new Node(ul, lr, RGBAPixel());
        //                        ul                lr
        node->NW = BuildNode(img, ul, make_pair(midX, midY)); // check 
        node->NE = BuildNode(img, make_pair(midX + 1, ul.second), make_pair(lr.first, midY)); // check 
        node->SW = BuildNode(img, make_pair(ul.first, midY + 1), make_pair(midX, lr.second)); // check 
        node->SE = BuildNode(img, make_pair(midX + 1, midY + 1), lr); // check 
        node->avg = calculateAverageColor(node);
        return node;
    }
}

void QTree::clearHelper(Node* &subRoot) {
    // not a null ptr
    if (subRoot) {
        clearHelper(subRoot->NW);
        clearHelper(subRoot->NE);
        clearHelper(subRoot->SW);
        clearHelper(subRoot->SE);
        delete subRoot;
        subRoot = nullptr;
    }
}

Node* QTree::copyHelper(Node* otherRoot) {
    if (otherRoot == nullptr) {
        return nullptr;
    }
    Node* newNode = new Node(otherRoot->upLeft, otherRoot->lowRight, otherRoot->avg);
    newNode->NW = copyHelper(otherRoot->NW);
    newNode->NE = copyHelper(otherRoot->NE);
    newNode->SW = copyHelper(otherRoot->SW);
    newNode->SE = copyHelper(otherRoot->SE);
    return newNode;
}

RGBAPixel QTree::calculateAverageColor(Node* node) {

    unsigned int totalR = 0, totalG = 0, totalB = 0;
    double totalA = 0.0;
    unsigned int totalArea = 0;
    
    vector<Node*> children;
    children.push_back(node->NW); // Top left 
    children.push_back(node->NE); // Top right 
    children.push_back(node->SW); // Bottom left
    children.push_back(node->SE); // Bottom right

    for (auto* child : children) {
        if (child != nullptr) {
            // single pixel 
            if (child->upLeft.first == child->lowRight.first && child->upLeft.second == child->lowRight.second) {
                unsigned int childArea = 1;
                totalR += child->avg.r * childArea;
                totalG += child->avg.g * childArea;
                totalB += child->avg.b * childArea;
                totalA += child->avg.a * (double) childArea;
                totalArea += childArea;
                continue;
            }
            // height is exactly 1
            else if (child->upLeft.second == child->lowRight.second) {
                unsigned int childArea = (child->lowRight.first - child->upLeft.first);
                totalR += child->avg.r * childArea;
                totalG += child->avg.g * childArea;
                totalB += child->avg.b * childArea;
                totalA += child->avg.a * (double) childArea;
                totalArea += childArea;
                continue;
            }
            // width is exactly 1
            else if (child->upLeft.first == child->lowRight.first) {
                unsigned int childArea = (child->lowRight.second - child->upLeft.second);
                totalR += child->avg.r * childArea;
                totalG += child->avg.g * childArea;
                totalB += child->avg.b * childArea;
                totalA += child->avg.a * (double) childArea;
                totalArea += childArea;
                continue;
            }
            else {
                unsigned int childArea = static_cast<unsigned int>((child->lowRight.first - child->upLeft.first) * (child->lowRight.second - child->upLeft.second));
                totalR += child->avg.r * childArea;
                totalG += child->avg.g * childArea;
                totalB += child->avg.b * childArea;
                totalA += child->avg.a * (double) childArea;
                totalArea += childArea;
            }
        }
    }

    unsigned char avgR = static_cast<unsigned char>(totalR / totalArea);
    unsigned char avgG = static_cast<unsigned char>(totalG / totalArea);
    unsigned char avgB = static_cast<unsigned char>(totalB / totalArea);
    double avgA = totalA / (double)totalArea;
    return RGBAPixel(avgR, avgG, avgB, avgA);
}

void QTree::renderHelper(Node* subRoot, PNG* image, unsigned int scale) const {
    if (subRoot == nullptr) return;

    if (isLeaf(subRoot)) {
        for (unsigned int x = subRoot->upLeft.first; x <= subRoot->lowRight.first; ++x) {
            for (unsigned int y = subRoot->upLeft.second; y <= subRoot->lowRight.second; ++y) {
                for (unsigned int dx = 0; dx < scale; ++dx) {
                    for (unsigned int dy = 0; dy < scale; ++dy) {
                        *image->getPixel(x * scale + dx, y * scale + dy) = subRoot->avg;
                    }
                }
            }
        }
    } else {
        renderHelper(subRoot->NW, image, scale);
        renderHelper(subRoot->NE, image, scale);
        renderHelper(subRoot->SW, image, scale);
        renderHelper(subRoot->SE, image, scale);
    }
}

bool QTree::isLeaf(Node* node) const {
    return node->NW == nullptr && node->NE == nullptr && 
           node->SW == nullptr && node->SE == nullptr;
}


void QTree::pruneHelper(Node* node, double tolerance) {
    if (node == nullptr) return;

    if (canPrune(node, node, tolerance)) {
        clearHelper(node->NW);
        clearHelper(node->NE);
        clearHelper(node->SW);
        clearHelper(node->SE);
    } else {
        pruneHelper(node->NW, tolerance);
        pruneHelper(node->NE, tolerance);
        pruneHelper(node->SW, tolerance);
        pruneHelper(node->SE, tolerance);
    }
}

bool QTree::canPrune(Node* root, Node* node, double tolerance) const {
    if (node == nullptr) return true;
    if (isLeaf(node)) return node->avg.distanceTo(root->avg) <= tolerance;
    return canPrune(root, node->NW, tolerance) && canPrune(root, node->NE, tolerance) &&
           canPrune(root, node->SW, tolerance) && canPrune(root, node->SE, tolerance);
}

void QTree::flipHorizontalHelper(Node* node, unsigned int right) {
    // base case
    if (node == nullptr) return;

    // Swap the horizontal positions
    unsigned int temp = node->upLeft.first;
    node->upLeft.first = right - (node->lowRight.first);
    node->lowRight.first = right - temp;
    
    flipHorizontalHelper(node->NW,right);
    flipHorizontalHelper(node->NE,right);
    flipHorizontalHelper(node->SW,right);
    flipHorizontalHelper(node->SE,right);

    // Swap child nodes horizontally
    Node* tempNode = new Node(make_pair(0,0), make_pair(0,0), RGBAPixel());
    tempNode = node->NW;
    node->NW = node->NE;
    node->NE = tempNode;
    tempNode = node->SW;
    node->SW = node->SE;
    node->SE = tempNode;
}

void QTree::rotateCCWHelper(Node* node) {
    if (node == nullptr) return;
    
    // Recursively rotate each child
    rotateCCWHelper(node->NW);
    rotateCCWHelper(node->NE);
    rotateCCWHelper(node->SW);
    rotateCCWHelper(node->SE);

    swap(node->upLeft.first, node->upLeft.second);
    swap(node->lowRight.first, node->lowRight.second);

    // Rearrange the children to reflect the rotation
    Node* temp = new Node(make_pair(0,0), make_pair(0,0), RGBAPixel());
    temp = node->NW;
    node->NW = node->NE;
    node->NE = node->SE;
    node->SE = node->SW;
    node->SW = temp;
}

void QTree::flipVerticalHelper(Node* node, unsigned int bottom) {
    // base case
    if (node == nullptr) return;

    // Swap the vertical positions
    unsigned int temp = node->upLeft.second;
    node->upLeft.second = bottom - (node->lowRight.second);
    node->lowRight.second = bottom - temp;
    
    flipVerticalHelper(node->NW, bottom);
    flipVerticalHelper(node->NE, bottom);
    flipVerticalHelper(node->SW, bottom);
    flipVerticalHelper(node->SE, bottom);

    // Swap child nodes vertically
    Node* tempNode = node->NW;
    node->NW = node->SW;
    node->SW = tempNode;
    tempNode = node->NE;
    node->NE = node->SE;
    node->SE = tempNode;
}
