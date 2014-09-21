#include "runner.h"

Runner::Runner(QObject *parent) :
    QObject(parent)
{
}

void Runner::Completed() { emit done(); }
