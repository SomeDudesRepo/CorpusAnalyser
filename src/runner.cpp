#include "runner.h"

namespace CorpusAnalyser
{

Runner::Runner(QObject *parent) :
    QObject(parent)
{
}

void Runner::Update(const int& updateType, const QString &message)
{
    emit update(updateType, message);
}

}  // namespace CorpusAnalyser
