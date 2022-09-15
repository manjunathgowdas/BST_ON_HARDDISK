#include<stdio.h>
#include"header.h"
#include<stdlib.h>
#define debug 0
#define LEFT 0  //This indicates whether a node is left child of parent or not
#define RIGHT 1  //This indicates whether a node is right child of parent or not
#define seek(offset) fseek(fp,offset*node_size+tree_size,SEEK_SET)//A macro for fseek function where,"offset" denotes node position in the file
#define read(temp) fread(&temp,node_size,1,fp)//A macro for fread function where,"temp" denotes the node to which the data is to be read
#define write(temp) fwrite(&temp,node_size,1,fp) //A macro for fwrite function where,"temp" denotes the node to which the data is to be written
#define read1(temp) fread(&temp,tree_size,1,fp)//A macro for fread fucntion to read the header(tree_t structure) of the file
#define write1(temp) fwrite(&temp,tree_size,1,fp)// A macro for fwrite function to write the header(tree_t structure) of the file
static long int node_size=sizeof(node_t),tree_size=sizeof(tree_t);
static int find_max(FILE* fp,int offset)//This function is used to find the total number of nodes present in the file at a given instant(including holes)
{
    node_t temp;
    if(offset==-1)//if offset is -1,then no node is present
    return 0;
    seek(offset);
    read(temp);
    return 1+find_max(fp,temp.left_offset)+find_max(fp,temp.right_offset);//recursively call the function for both left node and right node
}
FILE* init_tree(const char* filename)//This function checks whether the file is present or not .If not present,It creates one and initialises it with a header.If present it just returns the file pointer.
{
    FILE* fp;
    fp=fopen(filename,"r+");
   if(fp==NULL)
   { fp=fopen(filename,"w");
       tree_t header;
    header.free_head=-1;
    header.root=-1;
   write1(header);
    fclose(fp);
    fp=fopen(filename,"r+");
    }
return fp;
}
void close_tree(FILE *fp)//This function closes the file which is opened.
{
    fclose(fp);
}
void insert_key(int key, FILE *fp)//This function inserts the key in the valid node of the tree.It creates a new node,If their are no holes present in the tree.If present,it inserts the key into the hole.
{
    int max;//total number of nodes present in the tree(including holes)
    int offset,prev,free_node;
    tree_t temptree;
    fseek(fp,0,SEEK_SET);
    read1(temptree);
    free_node=temptree.free_head;
    if(temptree.root==-1 && temptree.free_head==-1)//if there is no tree,it assigns 0 to the max
    max=0;
    else
    {
    max=find_max(fp,temptree.root)+find_max(fp,temptree.free_head);//calls find_max to find the total number of nodes present in the tree
    }
        node_t tempnode;
        int left_or_right;//1 if the node is the right child of the parent,else 0
        offset=temptree.root;
        while(offset!=-1)//runs until the leaf node reached
        {prev=offset;//to store parents node
            seek(offset);
            read(tempnode);
            if(tempnode.key==key)//if key is already present in the tree ,return from the function
            return;
            else if(key<tempnode.key)//if key is less than the current node's key ,move left
            {
                offset=tempnode.left_offset;
                left_or_right=LEFT;
            }
            else//if key is greater than the current node's key,move right
            {
            offset=tempnode.right_offset;
            left_or_right=RIGHT;
            }
}
if(free_node!=-1)//if a hole is present ,use that to insert the key
{
    offset=free_node;
    seek(free_node);
    read(tempnode);
    temptree.free_head=tempnode.right_offset;//next free node is made the first free_node
    fseek(fp,0,0);
    write1(temptree);
}
else //if hole is not present or tree is not present ,create a new node
offset=max;
tempnode.key=key;
tempnode.right_offset=-1;   //initialising the node
tempnode.left_offset=-1;
if(temptree.root==-1)//if tree is not present
{
    tree_t tp;
    fseek(fp,0,SEEK_SET);
    read1(tp);
    tp.root=offset;//make the new node as root
    fseek(fp,0,0);
    write1(tp);
    
}
else//if tree is not empty then attach the node to the  valid side of parent depending on left_or_right
{
    node_t tp;
    seek(prev);
    read(tp);
     if(left_or_right)
            tp.right_offset=offset;
    else
            tp.left_offset=offset;
    seek(prev);
    write(tp);
}
seek(offset);//insert the node 
write(tempnode);
}
static int find_last_free_node(FILE*fp)//this function is used to find the last hole in the tree to attach the newly created node to it
{
    tree_t dummy;
    node_t temp;
    int prev;
    fseek(fp,0,0);
    read1(dummy);
    if(dummy.free_head==-1)//if no holes are present return -1
     return -1;
     seek(dummy.free_head);
     prev=dummy.free_head;
     read(temp);
     while(temp.right_offset!=-1)//traverse till the last hole
     {
         prev=temp.right_offset;
         seek(temp.right_offset);
         read(temp);
     }
    
     return prev;//return the offset of the last node
}
static void free_node(int offset,FILE*fp)//function to convert a node into  a hole and insert it at the end of the former last hole.
{int last_free_node;
    
        last_free_node=find_last_free_node(fp);//find the last hole
    node_t temp;
    seek(offset);
    read(temp);
    temp.left_offset=-1;
    temp.right_offset=-1;
    seek(offset);
    write(temp);
    if(last_free_node==-1)//if no hole is present put the new hole to the header of the file
            {
                        tree_t dum;
                     fseek(fp,0,0);
                         read1(dum);
                         dum.free_head=offset;
                         fseek(fp,0,0);
                         write1(dum);
                        }
                    else//if holes are present attach the new hole  to the former last node
                    {
                    seek(last_free_node);
                    read(temp);
                    temp.right_offset=offset;
                    seek(last_free_node);
                    write(temp);
                  
                    }
}
void delete_key(int key, FILE *fp)
{
    tree_t temp;
    int offset,prev,offset1,left_or_right;
    fseek(fp,0,SEEK_SET);
    read1(temp);
    if(temp.root==-1)//if tree is empty return
    return;
    offset=temp.root;
   prev=offset;//parent offset
    while(offset!=-1)//loops until the key is found 
    {
        node_t tempnode;
    seek(offset);
    read(tempnode);
        if(tempnode.key==key)//if key is equal to the node's key
        {
     if(tempnode.left_offset!=-1 && tempnode.right_offset!=-1)//if the node has both left and right child
            {
               node_t temp1;
                int offset1=tempnode.right_offset,prev1;
                seek(tempnode.right_offset);
                read(temp1);
                prev1=offset1;//parentoffset of the inorder predecesor
                while(temp1.left_offset!=-1)//loops till the inorder predecesor is found
                {
                    prev1=offset1;
                offset1=temp1.left_offset;
                seek(temp1.left_offset);
                read(temp1);
                 }
                if(temp1.right_offset!=-1 && prev1!=offset1)//if inorder predecesor  has a right child and if the right child of node to be deleted is not the inorder predecesor
                {
                    node_t dummy;
                    seek(prev1);
                    read(dummy);
                    dummy.left_offset=temp1.right_offset;//attach the right child of inorder predecesor to the left of its parent
                    seek(prev1);
                    write(dummy);
}
                else //if right child of the node to be deleted is the inorder predecesor or if the inorder predecesor does not have a right child
                {
                    node_t dummy ;
                    seek(prev1);
                    read(dummy);
                    dummy.left_offset=-1;//remove the inorder predecesor from its parent.
                    seek(prev1);
                    write(dummy);
                    }
                if(prev1==offset1)//if the right child of the node to be deleted is the inorder predecesor
            temp1.left_offset=tempnode.left_offset;//attach the left child of the node to be deleted to the left of inorder predecesor
            else//attach the left and right child of node to be deleted to the inorder predecesor
            {
                temp1.left_offset=tempnode.left_offset;
                temp1.right_offset=tempnode.right_offset;
            }
                seek(offset1);
                write(temp1);
                if(prev==offset)//if root is the node to be deleted
                {
                    tree_t dum;
                 fseek(fp,0,0);
                read1(dum);
                dum.root=offset1;//make the inorder predecesor as the root of the tree
                    fseek(fp,0,0);
                    write1(dum);
                    }
                else//if not ,Attach the inorder predecesor to the parent of the node to be deleted based on left_or_right
                {
                seek(prev);
                read(tempnode);
                if(left_or_right)
                tempnode.right_offset=offset1;
                else
                tempnode.left_offset=offset1;
                seek(prev);
                write(tempnode);
                }
                free_node(offset,fp);
            }
            else if(tempnode.left_offset!=-1)//if the node to be deleted has only left child
            {
                if(prev==offset)//if the root is the node to be deleted 
                {
                    tree_t nd;
                    fseek(fp,0,0);
                    read1(nd);
                    nd.root=tempnode.left_offset;//make the left child of that node as the root
                    fseek(fp,0,0);
                    write1(nd);
                }
                else//attach the left node to the parent of the node to be deleted based on left_or_right
                {
                node_t parentnode;
                seek(prev);
                read(parentnode);
                if(left_or_right)
                parentnode.right_offset=tempnode.left_offset;
            else
                parentnode.left_offset=tempnode.left_offset;
                seek(prev);
                write(parentnode);
                }
              free_node(offset,fp);
            }
            else if(tempnode.right_offset!=-1)//same as ,When only left child is present
            {
                 if(prev==offset)
                {
                    tree_t nd;
                    fseek(fp,0,0);
                    read1(nd);
                    nd.root=tempnode.right_offset;
                    fseek(fp,0,0);
                    write1(nd);
              }
                else
                {
                node_t parentnode;
                seek(prev);
                read(parentnode);
                if(left_or_right)
                parentnode.right_offset=tempnode.right_offset;
            else
                parentnode.left_offset=tempnode.right_offset;
                seek(prev);
                write(parentnode);
               }
              free_node(offset,fp);

            }
            else//if the node to be deleted has no child
            {
                tree_t nd;
                fseek(fp,0,0);
                read(nd);
                if(nd.root==offset)//if the root is the node to be deleted ,then assign root as -1
                {
                    nd.root=-1;
                   free_node(offset,fp);
                    fseek(fp,0,0);
                    write(nd);
                }
                else//remove the node from its parent
                {seek(prev);
                read(tempnode);
                if(left_or_right)
                tempnode.right_offset=-1;
                else
                tempnode.left_offset=-1;
                seek(prev);
                write(tempnode);
                free_node(offset,fp);
}
            }
            break;
        }
        else if(key<tempnode.key)//if key is less than the current node's key ,move left
        {
            prev=offset;
            offset=tempnode.left_offset;
            left_or_right=LEFT;
        }
        
        else//if key is greater than the current node's key, move right
        {
            prev=offset;
            left_or_right=RIGHT;
        offset=tempnode.right_offset;
        }
    }
}
static void inorder_helper(int offset,FILE*fp)//this function recursively calls itself to find the inorder of the node
{
    node_t temp;
    if(offset==-1)
    return;

    seek(offset);
    read(temp);
    inorder_helper(temp.left_offset,fp);
    printf("%d ",temp.key);//print the value of the current node
    inorder_helper(temp.right_offset,fp);
} 
static void preorder_helper(int offset,FILE* fp)//this helper function recursively calls itself to find the preorder of the node
{
    node_t temp;
    if(offset==-1)//if node is invalid return
    return;
    seek(offset);
    read(temp);
    printf("%d ",temp.key);//print the value of the current node
    preorder_helper(temp.left_offset,fp);
    preorder_helper(temp.right_offset,fp);
}
void display_inorder(FILE *fp)//helper unction to find the inorder of the node
{
    tree_t temp;
    fseek(fp,0,SEEK_SET);
    read1(temp);
    if(temp.root==-1)//if tree is empty return
    return;
    inorder_helper(temp.root,fp);
    printf("\n");

}
void display_preorder(FILE *fp)//function to find the preorder traversal
{
    tree_t temp;
    fseek(fp,0,SEEK_SET);
    read1(temp);
    if(temp.root==-1){
    printf("\n");
    return;
    }
    preorder_helper(temp.root,fp);
    printf("\n");
}

