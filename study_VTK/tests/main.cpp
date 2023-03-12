

extern int test_Cylinder(int argc, char* argv[]);
extern int test_PerspectiveTransform(int argc, char* argv[]);

int main(int argc, char* argv[]){

    int ret;
    ret = test_PerspectiveTransform(argc, argv);
    return ret;
}
