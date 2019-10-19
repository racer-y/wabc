# wabc
'wabc'库用作开发windows桌面程序，使用C++语言，以最小化的集合封装了常用的windows API，并包含一套消息处理框架。它源代码很小，不到500k，但很高效，使用的时候完全可以包含在自己的工程里。

'wabc'库的目录结构如下：

- wabc
  - dlib
  - include
  - lib
  - src
    -wabc

其中：
debug版本wabc.lib存放在dlib目录下，release版本的wabc.lib存放在lib目录下，引用'wabc'库的头文件'wabc.h'放在include目录，而源码则存放到src/wabc目录。

打开'wabc.h'头文件，注意这句：

#ifdef _DEBUG
#pragma comment(lib, "D:\\Racer\\wabc\\dlib\\wabc.lib")
#else
#pragma comment(lib, "D:\\Racer\\wabc\\lib\\wabc.lib")
#endif

这代码的意思是自动引入'wabc.lib'到自己的工程里，这里假设'wabc'库目录在"D:\Racer"下，若放在其它目录，请更改这里的值。

新建工程若直接引用'wabc.h'，必须添加'wabc/include'到自己的工程文件中。

在'wabc'库的默认配置里，代码生成的运行库选择的是“多线程调试(/MTd)”（Debug版本）和“多线程(/MT)”（Release版本），引用'wabc'库的工程必须保持和这个一致。当然，你也可以将它改成“多线程调试DLL(/MDd)”和“多线程(/MD)”。这两者并无本质上的区别，没有DLL的运行库生成的.exe文件会大一点，但它保证了.exe运行时不会出现找不到运行库的情况。

修改这选项的地方：右键单击工程“wabc”，选择“属性”=>“配置属性”=>“C/C++”=>“代码生成”=>“运行库”。

'wabc'库当前使用VS2013编译，当然，不一定需要2013，最低要求2008。若使用2013以下的IDE，可以新建一个'wabc'项目，应用程序类型选择“静态库(S)”，附加选项选择“空项目(E)”，生成新项目后将wabc库下的.h和.cpp文件添加到工程，再做如上相关修改。

相关资源：

wabc库设计原理：https://blog.csdn.net/racer_y/article/category/9415430
