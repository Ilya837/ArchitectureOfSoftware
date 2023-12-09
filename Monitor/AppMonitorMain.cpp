#include <windows.h>
#include "AppMonitor.h"

int main()
{
    Monitor m;
    int counter = 1;
    if(!m.init()) // in practice some sort of alarm should be raised here
        return 0;
    while (true) {
        if (!m.check()) {
            m.reset();
        }
        else {
            counter--;
            if (!counter) {
                counter = 1;
                m.Synchronise();
            }
        }

    }
}