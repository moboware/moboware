#include <boost/python.hpp>
#include <iostream>
#include <string>

using namespace boost::python;

class World
{
private:
  std::string name;

public:
  void set(std::string name) { this->name = name; }
  void greet() { std::cout << "hello, I am " << name << std::endl; }
};

typedef boost::shared_ptr<World> world_ptr;

BOOST_PYTHON_MODULE(hello)
{
  class_<World>("World").def("greet", &World::greet).def("set", &World::set);
};

int main(int argc, char* argv[])
{
  Py_Initialize();
  try {
    PyRun_SimpleString(
      "class Person:\n"
      "    def sayHi(self):\n"
      "        print 'hello from python'\n"
      "    def greetReset(self, instance):\n"
      "        instance.set('Python')\n");

    world_ptr worldObjectPtr(new World);
    worldObjectPtr->set("C++!");

    PyInit_hello();
    object o_main = object(handle<>(borrowed(PyImport_AddModule("__main__"))));
    object o_person_type = o_main.attr("Person");
    object o_person = o_person_type();
    object o_func1 = o_person.attr("sayHi");
    o_func1();
    object o_func2 = o_person.attr("greetReset");
    o_func2(boost::python::ptr(worldObjectPtr.get()));
    worldObjectPtr->greet();
  } catch (error_already_set) {
    PyErr_Print();
  }

  Py_Finalize();
  return 0;
}