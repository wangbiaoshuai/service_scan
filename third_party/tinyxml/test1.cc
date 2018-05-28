#include <tinyxml.h>
#include <string>

/*!
*  /brief 打印xml文件。
*
*  /param XmlFile xml文件全路径。
*  /return 是否成功。true为成功，false表示失败。
*/
bool PaintXml(std::string XmlFile)
{
    // 定义一个TiXmlDocument类指针
    TiXmlDocument pDoc(XmlFile.c_str());
    pDoc.LoadFile();
    pDoc.Print();
    return true;
}

int main()
{
    std::string xmlpath("./mytest.xml");
    PaintXml(xmlpath);
    return 0;
}
