/*
* BTree.cc - implementation of methods of class BTree as declared in BTree.h.
*
* Several of the methods in this file must be implemented by students for
* cs222 project 4
*
* If this file is compiled with the symbol PROFESSOR_VERSION defined,
* it expects a file of that name to exist, and uses the code from
* that file in place of selected dummy methods left to be written by students.
*
* Copyright (c) 2001, 2002, 2003 - Russell C. Bjork
* Trivial changes 2016 - Russ Tuck
* Edited by Jonathan Manos & Alonzo Ouzts - 4/15/16
*
* Implemented lookup and insert
*/

#include <iostream>
using std::cout;

#include "BTree.h"
#include "BTreeFile.h"
#include "BTreeBlock.h"


BTree::BTree(string name)
: _file(* new BTreeFile(name))
{ }

#ifndef PROFESSOR_VERSION

void BTree::insert(string key, string value)
{

    //Instantiates BlockNumbers and BTreeBlocks
    BTreeFile::BlockNumber blockNumber;
    BTreeFile::BlockNumber splitBlockNumber;
    BTreeFile::BlockNumber childBlockNumber;
    BTreeFile::BlockNumber blockNumberNewRoot;
    BTreeBlock rootBlock;
    BTreeBlock currentBlock;
    BTreeBlock splitBlock;

    //Sets the root as the currentBlock
    blockNumber = _file.getRoot();
    _file.getBlock(blockNumber, currentBlock);


    int index = currentBlock.getPosition(key);
    int numberOfKeys = 0;
    int splitIndex = 0;

    //If there is no root block, make the root block and insert
    //the given key and value
    //else if there is a root proceed trying to find the position
    //to insert the given key and value
    if(blockNumber == 0)
    {

        blockNumber = _file.allocateBlock();

        currentBlock.insert(index, key, value, 0);
        currentBlock.setChild(index, 0);
        _file.putBlock(blockNumber, currentBlock);
        _file.setRoot(blockNumber);
    }
    else
    {
        //while loop that keeps going until the leaf block is found that
        //the key needs to be inserted on
        while(!currentBlock.isLeaf())
        {
            blockNumber = currentBlock.getChild(index);
            _file.getBlock(blockNumber, currentBlock);
            index = currentBlock.getPosition(key);
        }

        //inserts the key and value at the proper position in the block
        currentBlock.insert(index, key, value, 0);
        currentBlock.setChild(index, 0);

        //If a split is not needed the block is put into the file and the
        //insert process is over
        //else a split is needed
        if(!currentBlock.splitNeeded())
        {
            _file.putBlock(blockNumber, currentBlock);
        }
        else
        {
            //splits the current block and puts the left side in its same spot
            currentBlock.split(key, value, splitBlock);
            _file.putBlock(blockNumber, currentBlock);

            //takes the right side and allocates a new block to put it in
            splitBlockNumber = _file.allocateBlock();
            _file.putBlock(splitBlockNumber, splitBlock);

            //store the block number that the original split happened in
            childBlockNumber = blockNumber;

            //set block number to the root and get the currentBlock back to the
            //root block
            blockNumber = _file.getRoot();
            _file.getBlock(blockNumber,currentBlock);
            index = currentBlock.getPosition(key);

            //since split was done, there is a key and value that needs
            //promoting
            bool stillNeedsPromoting = true;

            //while loop that keeps trying to insert promoted keys and values
            //until an insert doesn't split
            while(stillNeedsPromoting)
            {
                //If the block that split before was the root
                if(_file.getRoot() == childBlockNumber)
                {
                    //allocates a new block to be used as the root
                    blockNumberNewRoot = _file.allocateBlock();
                    _file.getBlock(blockNumberNewRoot, rootBlock);
                    index = rootBlock.getPosition(key);

                    //inserts the promoted key and value into the new root
                    //block and sets its child to the two halves of the
                    //previous root, and then sets itself as the new root
                    rootBlock.setChild(0, blockNumber);
                    rootBlock.insert(index, key, value, splitBlockNumber);
                    _file.putBlock(blockNumberNewRoot, rootBlock);
                    _file.setRoot(blockNumberNewRoot);

                    //still needs promoting set to false, no promoted value
                    //insert is done
                    stillNeedsPromoting = false;
                }
                else if(currentBlock.getChild(index) == childBlockNumber)
                {
                    //if the current block is the parent of the split block
                    //insert the promoted key and value
                    index = currentBlock.getPosition(key);
                    currentBlock.insert(index, key, value, splitBlockNumber);

                    //if there is no split needed, put the block in the file
                    // and still needs promoting is false so inserting is done
                    if(!currentBlock.splitNeeded())
                    {
                        _file.putBlock(blockNumber, currentBlock);
                        stillNeedsPromoting = false;
                    }
                    else
                    {
                        //else another split needs to be done
                        currentBlock.split(key, value, splitBlock);
                        _file.putBlock(blockNumber, currentBlock);

                        splitBlockNumber = _file.allocateBlock();
                        _file.putBlock(splitBlockNumber, splitBlock);

                        childBlockNumber = blockNumber;

                        //sets the current block back to the root block and
                        //still needs promoting set to true so it runs
                        //through trying to insert again
                        _file.getBlock(_file.getRoot(),currentBlock);
                        stillNeedsPromoting = true;
                    }
                }
                else
                {
                    //else the currentBlock is not the parent of the split
                    //or the root where the split happened so the block
                    //is set to the next logical child of the key
                    blockNumber = currentBlock.getChild(index);
                    _file.getBlock(blockNumber, currentBlock);
                    index = currentBlock.getPosition(key);
                }
            }
        }
    }
}

bool BTree::lookup(string key, string & value) const
{

    //Instantiate BlockNumber and BTreeBlock
    BTreeFile::BlockNumber blockNumber = _file.getRoot();
    BTreeBlock currentBlock;
    _file.getBlock(blockNumber, currentBlock);

    //Set int value as the index of the position of the key
    int index = currentBlock.getPosition(key);

    //While loop that returns true if the currentBlock contains the key
    //and setting the currentBlock to the child at the given index.
    //loop stops if blockNumber is 0, meaning it reached the end of a leaf.
    while(blockNumber != 0)
    {

        if(currentBlock.getKey(index) == key)
        {
            value = currentBlock.getValue(index);
            return true;
        }
        else
        {
            blockNumber = currentBlock.getChild(index);
            _file.getBlock(blockNumber, currentBlock);
            index = currentBlock.getPosition(key);
        }

    }
    return false;
}

bool BTree::remove(string key)
{
    return false; // Student code goes here - remove this line
}

#else

#define QUOTE(Q) #Q
#define INCLUDE_NAME(X) QUOTE(X)
#include INCLUDE_NAME(PROFESSOR_VERSION)

#endif

void BTree::print() const
{
    cout << "BTree in file ";
    _file.printHeaderInfo();
    cout << endl;

    BTreeFile::BlockNumber root = _file.getRoot();
    if (root == 0)
    cout << "Empty tree" << endl;
    else
    _file.printBlock(root, true, 1);
}

void BTree::print(BTreeFile::BlockNumber blockNumber) const
{
    _file.printBlock(blockNumber, false, 1);
}

BTree::~BTree()
{
    delete (& _file);
}
