#include<stdio.h>
#include"header.h"
int main()
{
    FILE *fp=init_tree("pn.dat");
insert_key(10,fp);
insert_key(20,fp);
display_inorder(fp);
delete_key(10,fp);
delete_key(20,fp);
display_inorder(fp);
insert_key(40,fp);
display_inorder(fp);
close_tree(fp);
    return 0;
}