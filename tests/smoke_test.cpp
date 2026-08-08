#include "core/Config.h"
#include "core/Schema.h"

#include <cassert>

int main() {
    datastorage::Config config("config/cluster.ini");
    datastorage::SchemaLoader schema("config/schema.ini");

    assert(config.cluster().nodeCount == 3);
    assert(schema.schema().keyField == "id");
    assert(schema.schema().fields.size() == 3);
    return 0;
}
