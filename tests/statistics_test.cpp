#include "statistics/Statistics.h"

#include <cassert>

int main() {
    datastorage::StatisticsCollector stats(3);

    stats.recordRead(1);
    stats.recordRead(1, 2u);
    stats.recordStored(1);
    stats.recordTransferred(2, 3u);
    stats.recordReceived(2, 2u);
    stats.recordValid(3u);
    stats.recordInvalid(1u);
    stats.recordDuplicate(2u);

    const datastorage::ClusterStatistics snapshot = stats.snapshot();

    assert(snapshot.totalRecordsRead == 3u);
    assert(snapshot.validRecords == 3u);
    assert(snapshot.invalidRecords == 1u);
    assert(snapshot.duplicateRecords == 2u);
    assert(snapshot.recordsLoaded == 3u);
    assert(snapshot.recordsStored == 1u);
    assert(snapshot.recordsTransferred == 3u);
    assert(snapshot.recordsReceived == 2u);
    assert(snapshot.perNode.size() == 3u);
    assert(snapshot.perNode[0].recordsLoaded == 1u);
    assert(snapshot.perNode[1].recordsReceived == 2u);
    assert(snapshot.perNode[1].recordsLoaded == 2u);
    assert(snapshot.perNode[1].recordsTransferred == 3u);

    return 0;
}
