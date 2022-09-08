import React from "react";
import { $L } from "cfx/common/services/intl/l10n";
import { Button } from "cfx/ui/Button/Button";
import { ButtonBar } from "cfx/ui/Button/ButtonBar";
import { Title } from "cfx/ui/Title/Title";
import { observer } from "mobx-react-lite";
import { useLocation } from "react-router-dom";
import { ServersListType } from "cfx/common/services/servers/lists/types";
import { useService } from "cfx/base/servicesContainer";
import { IServersService } from "cfx/common/services/servers/servers.service";
import { Icons } from "cfx/ui/Icons";
import { TextColor } from "cfx/ui/Text/Text";

export const SERVER_LIST_DESCRIPTORS: Record<string, { title: React.ReactNode, icon: React.ReactNode, to: string, color: TextColor }> = {
  [ServersListType.All]: {
    title: $L('#ServerList_Browse'),
    icon: Icons.serversListAll,
    color: 'inherit',
    to: '/servers',
  },
  [ServersListType.Supporters]: {
    title: $L('#ServerList_Premium'),
    icon: Icons.serversListSupporters,
    color: 'warning',
    to: '/servers/premium',
  },
  [ServersListType.History]: {
    title: $L('#ServerList_History'),
    icon: Icons.serversListHistory,
    color: 'teal',
    to: '/servers/history',
  },
  [ServersListType.Favorites]: {
    title: $L('#ServerList_Favorites'),
    icon: Icons.serversListFavorites,
    color: 'primary',
    to: '/servers/favorites',
  },
};

export const ListTypeTabs = observer(function ListTypeTabs() {
  const { pathname } = useLocation();

  const ServersService = useService(IServersService);

  const nodes = ServersService.listTypes.map((type) => {
    if (!SERVER_LIST_DESCRIPTORS[type]) {
      return null;
    }

    const { title, icon, to } = SERVER_LIST_DESCRIPTORS[type];

    return (
      <Title key={type} title={title}>
        <Button
          key={type}
          to={to}
          size="large"
          icon={icon}
          theme={pathname === to ? 'primary' : 'default'}
        />
      </Title>
    );
  });

  return (
    <ButtonBar>
      {nodes}
    </ButtonBar>
  );
});
