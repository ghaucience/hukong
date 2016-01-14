lib-linux
=========

**这是什么**

封装了linux下的一些功能模块，其中包括之前的SimpleNet。

* SimpleNet

    是一个Windows/Linux下的C++网络库，简单封装了socket API。

    在写这个库之前，我看了很多的C++网络库，但简洁好用的很少。所以我参考[Etwork](http://www.enchantedage.com/etwork)和[Kevin Lynx的kl_net](http://www.cppblog.com/kevinlynx/archive/2008/05/28/51412.html)写了个精简版，采用简单的Select模型和同步方式（去掉复杂的多线程操作），只支持TCP的服务器和客户端模式。
