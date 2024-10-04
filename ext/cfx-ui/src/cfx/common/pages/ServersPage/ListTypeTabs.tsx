import {
  ButtonBar,
  Icons,
  Tabular,
  Title,
  Icon,
} from '@cfx-dev/ui-components';
import { observer } from 'mobx-react-lite';
import React from 'react';
import { useLocation, useNavigate } from 'react-router-dom';

import { useService } from 'cfx/base/servicesContainer';
import { useEventHandler } from 'cfx/common/services/analytics/analytics.service';
import { EventActionNames, ElementPlacements } from 'cfx/common/services/analytics/types';
import { $L } from 'cfx/common/services/intl/l10n';
import { LocaleKeyOrString, LocaleKey } from 'cfx/common/services/intl/types';
import { ServersListType } from 'cfx/common/services/servers/lists/types';
import { IServersService } from 'cfx/common/services/servers/servers.service';
import { LinkButton } from 'cfx/ui/Button/LinkButton';

interface ServerListDescriptor {
  titleKey: LocaleKeyOrString<LocaleKey>;
  icon: React.ReactNode;
  to: string;
  color: React.ComponentProps<typeof Icon>['color'];
}

export const SERVER_LIST_DESCRIPTORS: Record<string, ServerListDescriptor> = {
  [ServersListType.All]: {
    titleKey: '#ServerList_Browse',
    icon: Icons.serversListAll,
    color: 'inherit',
    to: '/servers',
  },
  [ServersListType.Supporters]: {
    titleKey: '#ServerList_Premium',
    icon: Icons.serversListSupporters,
    color: 'warning',
    to: '/servers/premium',
  },
  [ServersListType.History]: {
    titleKey: '#ServerList_History',
    icon: Icons.serversListHistory,
    color: 'teal',
    to: '/servers/history',
  },
  [ServersListType.Favorites]: {
    titleKey: '#ServerList_Favorites',
    icon: Icons.serversListFavorites,
    color: 'primary',
    to: '/servers/favorites',
  },
};

export const ListTypeTabs = observer(function ListTypeTabs() {
  const {
    pathname,
  } = useLocation();
  const navigate = useNavigate();
  const ServersService = useService(IServersService);
  const eventHandler = useEventHandler();

  const handleClick = React.useCallback(
    (descriptor: ServerListDescriptor) => {
      eventHandler({
        action: EventActionNames.AccountInfoCTA,
        properties: {
          element_placement: ElementPlacements.Nav,
          text: descriptor.titleKey,
          link_url: descriptor.to,
        },
      });

      navigate(descriptor.to);
    },
    [eventHandler, navigate],
  );

  return (
    <Tabular.Root size="large">
      {ServersService.listTypes.map((serverListType) => {
        const descriptor = SERVER_LIST_DESCRIPTORS[serverListType];

        if (!descriptor) {
          return null;
        }

        return (
          <Title key={descriptor.to} title={$L(descriptor.titleKey)}>
            <Tabular.Item
              active={pathname === descriptor.to}
              icon={descriptor.icon}
              onClick={() => handleClick(descriptor)}
            />
          </Title>
        );
      })}
    </Tabular.Root>
  );
});

export const ListTypeTabs2 = observer(function ListTypeTabs2() {
  const {
    pathname,
  } = useLocation();

  const ServersService = useService(IServersService);

  const nodes = ServersService.listTypes.map((type) => {
    if (!SERVER_LIST_DESCRIPTORS[type]) {
      return null;
    }

    const {
      titleKey,
      icon,
      to,
    } = SERVER_LIST_DESCRIPTORS[type];

    return (
      <Title key={type} title={$L(titleKey)}>
        <LinkButton
          key={type}
          to={to}
          size="large"
          icon={icon}
          theme={pathname === to
            ? 'primary'
            : 'default'}
        />
      </Title>
    );
  });

  return (
    <ButtonBar>{nodes}</ButtonBar>
  );
});
