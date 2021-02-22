int clock_tests();
int channel_tests();


int main(int argc, char* argv[])
{
    if (clock_tests()) return 1;
    if (channel_tests()) return 1;
    return 0;
}
