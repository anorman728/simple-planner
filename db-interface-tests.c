#include "db-interface.h"

int main()
{
    db_interface_initialize("./testing.db");
    db_interface_finalize();
}
