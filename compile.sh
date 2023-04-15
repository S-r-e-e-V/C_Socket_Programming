clear
gcc serverDir/server.c -o serverDir/server
gcc serverDir/mirror.c -o serverDir/mirror
gcc ClientDir/client.c -o ClientDir/client

cd ~
rm -rf testingFolder
mkdir testingFolder
cd testingFolder
echo "abc" > a.txt
echo "abc" > b.txt
echo "abc" > c.pdf
echo "abc" > d.pdf
echo "abc" > e.png
