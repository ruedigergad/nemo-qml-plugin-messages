/* Copyright (C) 2012 John Brooks <john.brooks@dereferenced.net>
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Nemo Mobile nor the names of its contributors
 *     may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "clienthandler.h"
#include "conversationchannel.h"
#include "groupmanager.h"

#include <TelepathyQt/ChannelClassSpec>
#include <TelepathyQt/ReceivedMessage>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/ChannelRequest>
#include <TelepathyQt/Account>
#include <TelepathyQt/ClientRegistrar>

using namespace Tp;

ClientHandler::ClientHandler()
    : AbstractClientHandler(ChannelClassSpec::textChat()), manager(0)
{
    const QDBusConnection &dbus = QDBusConnection::sessionBus();
    registrar = ClientRegistrar::create(dbus);
    AbstractClientPtr handler = AbstractClientPtr(this);
    registrar->registerClient(handler, "qmlmessages");
}

ClientHandler::~ClientHandler()
{
}

void ClientHandler::setGroupManager(GroupManager *g)
{
    manager = g;
}

bool ClientHandler::bypassApproval() const
{
    return true;
}

void ClientHandler::handleChannels(const MethodInvocationContextPtr<> &context, const AccountPtr &account,
                                   const ConnectionPtr &connection, const QList<ChannelPtr> &channels,
                                   const QList<ChannelRequestPtr> &requestsSatisfied, const QDateTime &userActionTime,
                                   const HandlerInfo &handlerInfo)
{
    if (!manager) {
        qWarning() << "handleChannels has no group manager instance";
        context->setFinished();
        return;
    }

    foreach (const ChannelPtr &channel, channels) {
        QVariantMap properties = channel->immutableProperties();
        QString targetId = properties.value(TP_QT_IFACE_CHANNEL + QLatin1String(".TargetID")).toString();

        if (targetId.isEmpty()) {
            qWarning() << "handleChannels cannot get TargetID for channel";
            continue;
        }

        ConversationChannel *g = manager->getConversation(account->objectPath(), targetId);
        if (!g) {
            qWarning() << "handleChannels cannot create ConversationChannel";
            continue;
        }

        g->setChannel(channel);
    }

    context->setFinished();
}

