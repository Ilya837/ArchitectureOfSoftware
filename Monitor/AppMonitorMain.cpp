#include <windows.h>
#include "AppMonitor.h"

int main()
{
    Monitor m;
    if(!m.init()) // in practice some sort of alarm should be raised here
        return 0;
    while (true) {
        if (!m.check()) {
            m.reset();
            m.init();
        }

    }
}