/* This file is part of the KDE project
   Copyright (C) 2004 Dag Andersen <danders@get2net.dk>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "kptcommand.h"
#include "kptpart.h"
#include "kptproject.h"
#include "kptcalendar.h"

#include <kdebug.h>
#include <klocale.h>


KPTCalendarAddCmd::KPTCalendarAddCmd(KPTPart *part, KPTProject *project,KPTCalendar *cal, QString name)
    : KNamedCommand(name),
      m_part(part),
      m_project(project),
      m_cal(cal),
      m_added(false) {
    cal->setDeleted(true);  
    //kdDebug()<<k_funcinfo<<cal->name()<<endl;
}

void KPTCalendarAddCmd::execute() {
    if (!m_added && m_project) {
        m_project->addCalendar(m_cal);
        m_added = true;
    }
    m_cal->setDeleted(false);
    if (m_part)
        m_part->setCommandType(0);
    //kdDebug()<<k_funcinfo<<m_cal->name()<<" added to: "<<m_project->name()<<endl;
}

void KPTCalendarAddCmd::unexecute() {
    m_cal->setDeleted(true);
    if (m_part)
        m_part->setCommandType(0);
    //kdDebug()<<k_funcinfo<<m_cal->name()<<endl;
}

KPTCalendarDeleteCmd::KPTCalendarDeleteCmd(KPTPart *part, KPTCalendar *cal, QString name)
    : KNamedCommand(name),
      m_part(part),
      m_cal(cal) {
}

void KPTCalendarDeleteCmd::execute() {
    m_cal->setDeleted(true);
    if (m_part)
        m_part->setCommandType(0);
}

void KPTCalendarDeleteCmd::unexecute() {
    m_cal->setDeleted(false);
    if (m_part)
        m_part->setCommandType(0);
}


KPTNodeAddCmd::KPTNodeAddCmd(KPTPart *part, KPTProject *project, KPTNode *node, KPTNode *position,  QString name)
    : KNamedCommand(name),
      m_part(part),
      m_project(project),
      m_node(node),
      m_position(position),
      m_added(false) {
    node->setDeleted(true);
}

void KPTNodeAddCmd::execute() {
    m_node->setDeleted(false);
    if (m_part)
        m_part->setCommandType(1);
}

void KPTNodeAddCmd::unexecute() {
    m_node->setDeleted(true);
    if (m_part)
        m_part->setCommandType(1);
}


KPTNodeDeleteCmd::KPTNodeDeleteCmd(KPTPart *part, KPTNode *node, QString name)
    : KNamedCommand(name),
      m_part(part),
      m_node(node) {
}

void KPTNodeDeleteCmd::execute() {
    m_node->setDeleted(true);
    if (m_part)
        m_part->setCommandType(1);
}

void KPTNodeDeleteCmd::unexecute() {
    m_node->setDeleted(false);
    if (m_part)
        m_part->setCommandType(1);
}


KPTTaskAddCmd::KPTTaskAddCmd(KPTPart *part, KPTProject *project, KPTNode *node, KPTNode *position,  QString name)
    : KPTNodeAddCmd(part, project, node, position, name) {
}

void KPTTaskAddCmd::execute() {
    if (!m_added && m_project) {
        m_project->addTask(m_node, m_position);
        m_added = true;
    }
    KPTNodeAddCmd::execute();
}

KPTSubtaskAddCmd::KPTSubtaskAddCmd(KPTPart *part, KPTProject *project, KPTNode *node, KPTNode *position,  QString name)
    : KPTNodeAddCmd(part, project, node, position, name) {   
}

void KPTSubtaskAddCmd::execute() {
    if (!m_added && m_project) {
        m_project->addSubTask(m_node, m_position);
        m_added = true;
    }
    KPTNodeAddCmd::execute();
}


KPTNodeModifyNameCmd::KPTNodeModifyNameCmd(KPTPart *part, KPTNode &node, QString nodename, QString name)
    : KNamedCommand(name),
      m_part(part),
      m_node(node),
      newName(nodename),
      oldName(node.name()) {

}
void KPTNodeModifyNameCmd::execute() {
    m_node.setName(newName);
    if (m_part)
        m_part->setCommandType(0);
}
void KPTNodeModifyNameCmd::unexecute() {
    m_node.setName(oldName);
    if (m_part)
        m_part->setCommandType(0);
}

KPTNodeModifyLeaderCmd::KPTNodeModifyLeaderCmd(KPTPart *part, KPTNode &node, QString leader, QString name)
    : KNamedCommand(name),
      m_part(part),
      m_node(node),
      newLeader(leader),
      oldLeader(node.leader()) {

}
void KPTNodeModifyLeaderCmd::execute() {
    m_node.setLeader(newLeader);
    if (m_part)
        m_part->setCommandType(0);
}
void KPTNodeModifyLeaderCmd::unexecute() {
    m_node.setLeader(oldLeader);
    if (m_part)
        m_part->setCommandType(0);
}

KPTNodeModifyDescriptionCmd::KPTNodeModifyDescriptionCmd(KPTPart *part, KPTNode &node, QString description, QString name)
    : KNamedCommand(name),
      m_part(part),
      m_node(node),
      newDescription(description),
      oldDescription(node.description()) {

}
void KPTNodeModifyDescriptionCmd::execute() {
    m_node.setDescription(newDescription);
    if (m_part)
        m_part->setCommandType(0);
}
void KPTNodeModifyDescriptionCmd::unexecute() {
    m_node.setDescription(oldDescription);
    if (m_part)
        m_part->setCommandType(0);
}

KPTNodeModifyConstraintCmd::KPTNodeModifyConstraintCmd(KPTPart *part, KPTNode &node, KPTNode::ConstraintType c, QString name)
    : KNamedCommand(name),
      m_part(part),
      m_node(node),
      newConstraint(c),
      oldConstraint(static_cast<KPTNode::ConstraintType>(node.constraint())) {

}
void KPTNodeModifyConstraintCmd::execute() {
    m_node.setConstraint(newConstraint);
    if (m_part)
        m_part->setCommandType(1);
}
void KPTNodeModifyConstraintCmd::unexecute() {
    m_node.setConstraint(oldConstraint);
    if (m_part)
        m_part->setCommandType(1);
}

KPTNodeModifyConstraintTimeCmd::KPTNodeModifyConstraintTimeCmd(KPTPart *part, KPTNode &node, QDateTime dt, QString name)
    : KNamedCommand(name),
      m_part(part),
      m_node(node),
      newTime(dt),
      oldTime(node.constraintTime()) {

}
void KPTNodeModifyConstraintTimeCmd::execute() {
    m_node.setConstraintTime(newTime);
    if (m_part)
        m_part->setCommandType(1);
}
void KPTNodeModifyConstraintTimeCmd::unexecute() {
    m_node.setConstraintTime(oldTime);
    if (m_part)
        m_part->setCommandType(1);
}

KPTNodeIndentCmd::KPTNodeIndentCmd(KPTPart *part, KPTNode &node, QString name)
    : KNamedCommand(name),
      m_part(part),
      m_node(node), 
      m_newparent(0), 
      m_newindex(-1) {

}
void KPTNodeIndentCmd::execute() {
    m_oldparent = m_node.getParent();
    m_oldindex = m_oldparent->findChildNode(&m_node);
    KPTProject *p = dynamic_cast<KPTProject *>(m_node.projectNode());
    if (p && p->indentTask(&m_node)) {
        m_newparent = m_node.getParent();
        m_newindex = m_newparent->findChildNode(&m_node);
        m_node.setParent(m_newparent);
    }
    if (m_part)
        m_part->setCommandType(1);
}
void KPTNodeIndentCmd::unexecute() {
    if (m_newindex != -1) {
        m_newparent->delChildNode(m_newindex, false);
        m_oldparent->insertChildNode(m_oldindex, &m_node);
        m_node.setParent(m_oldparent);
        m_newindex = -1;
    }
    if (m_part)
        m_part->setCommandType(1);
}

KPTNodeUnindentCmd::KPTNodeUnindentCmd(KPTPart *part, KPTNode &node, QString name)
    : KNamedCommand(name),
      m_part(part),
      m_node(node), 
      m_newparent(0),  
      m_newindex(-1) {
}
void KPTNodeUnindentCmd::execute() {
    m_oldparent = m_node.getParent();
    m_oldindex = m_oldparent->findChildNode(&m_node);
    KPTProject *p = dynamic_cast<KPTProject *>(m_node.projectNode());
    if (p && p->unindentTask(&m_node)) {
        m_newparent = m_node.getParent();
        m_newindex = m_newparent->findChildNode(&m_node);
        m_node.setParent(m_newparent);
    }
    if (m_part)
        m_part->setCommandType(1);
}
void KPTNodeUnindentCmd::unexecute() {
    if (m_newindex != -1) {
        m_newparent->delChildNode(m_newindex, false);
        m_oldparent->insertChildNode(m_oldindex, &m_node);
        m_node.setParent(m_oldparent);
        m_newindex = -1;
    }
    if (m_part)
        m_part->setCommandType(1);
}

KPTNodeMoveUpCmd::KPTNodeMoveUpCmd(KPTPart *part, KPTNode &node, QString name)
    : KNamedCommand(name),
      m_part(part),
      m_node(node), 
      m_newindex(-1) {
}
void KPTNodeMoveUpCmd::execute() {
    m_oldindex = m_node.getParent()->findChildNode(&m_node);
    KPTProject *p = dynamic_cast<KPTProject *>(m_node.projectNode());
    if (p && p->moveTaskUp(&m_node)) {
        m_newindex = m_node.getParent()->findChildNode(&m_node);
    }
    if (m_part)
        m_part->setCommandType(0);
}
void KPTNodeMoveUpCmd::unexecute() {
    if (m_newindex != -1) {
        m_node.getParent()->delChildNode(m_newindex, false);
        m_node.getParent()->insertChildNode(m_oldindex, &m_node);
        m_newindex = -1;
    }
    if (m_part)
        m_part->setCommandType(0);
}

KPTNodeMoveDownCmd::KPTNodeMoveDownCmd(KPTPart *part, KPTNode &node, QString name)
    : KNamedCommand(name),
      m_part(part),
      m_node(node), 
      m_newindex(-1) {
}
void KPTNodeMoveDownCmd::execute() {
    m_oldindex = m_node.getParent()->findChildNode(&m_node);
    KPTProject *p = dynamic_cast<KPTProject *>(m_node.projectNode());
    if (p && p->moveTaskDown(&m_node)) {
        m_newindex = m_node.getParent()->findChildNode(&m_node);
    }
    if (m_part)
        m_part->setCommandType(0);
}
void KPTNodeMoveDownCmd::unexecute() {
    if (m_newindex != -1) {
        m_node.getParent()->delChildNode(m_newindex, false);
        m_node.getParent()->insertChildNode(m_oldindex, &m_node);
        m_newindex = -1;
    }
    if (m_part)
        m_part->setCommandType(0);
}

