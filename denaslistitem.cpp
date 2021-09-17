#include "denaslistitem.h"

DenasListItem::DenasListItem(){
    associatedList = NULL;
    associatedDisplay = NULL;
    modeFlag = "";
}

string DenasListItem::getFlag(){return modeFlag;}

void DenasListItem::setFlag(string f){modeFlag = f;}

void DenasListItem::setAssocList(QListWidget *l) {associatedList = l;}
QListWidget* DenasListItem::getAssocList() {return associatedList;}
