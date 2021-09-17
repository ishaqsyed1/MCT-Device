#ifndef DENASLISTITEM_H
#define DENASLISTITEM_H

#include <QWidget>
#include <QListWidget>
#include <QListWidgetItem>
#include <string>

using namespace std;

class DenasListItem : public QListWidgetItem
{
    //Q_OBJECT
public:
    DenasListItem();
    void setAssocList(QListWidget*);
    QListWidget* getAssocList();
    string getFlag();
    void setFlag(string);

private:
    QListWidget *associatedList;
     QScreen *associatedDisplay;
     string modeFlag;

signals:

};

#endif // DENASLISTITEM_H
