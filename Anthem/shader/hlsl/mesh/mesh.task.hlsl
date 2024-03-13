
struct Dummy
{
    uint dummy;
};

groupshared Dummy data;

[numthreads(1, 1, 1)]
void main()
{
    DispatchMesh(3, 1, 1, data);
}