#ifndef RUNNER_H
#define RUNNER_H

#include <QObject>

class Runner : public QObject
{
    Q_OBJECT
public:
    explicit Runner(QObject *parent = 0);
    void Completed();

signals:
    void done();

public slots:

};

#endif // RUNNER_H
