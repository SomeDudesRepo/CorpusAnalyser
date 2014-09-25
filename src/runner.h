#ifndef RUNNER_H
#define RUNNER_H

#include <QObject>

class Runner : public QObject
{
    Q_OBJECT
public:
    explicit Runner(QObject *parent = 0);
    void Update(const int& updateType, const QString& message);

signals:
    void update(const int& updateType, const QString& message);

public slots:

};

#endif // RUNNER_H
