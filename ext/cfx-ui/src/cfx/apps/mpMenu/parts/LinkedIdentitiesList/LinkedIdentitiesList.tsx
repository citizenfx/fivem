import {
  BrandIcon,
  Flex,
  Title,
} from '@cfx-dev/ui-components';
import { observer } from 'mobx-react-lite';
import React from 'react';
import { FaDiscord, FaRoad, FaSteam, FaXbox } from 'react-icons/fa';

import { useService } from 'cfx/base/servicesContainer';
import { $L } from 'cfx/common/services/intl/l10n';
import { ILinkedIdentitiesService } from 'cfx/common/services/linkedIdentities/linkedIdentities.service';

import { IConvarService, KnownConvars } from '../../services/convars/convars.service';
import { ILinkedIdentity, LinkedIdentityProvider } from '../../services/linkedIdentities/types';

export const LinkedIdentitiesList = observer(function LinkedIdentitiesList() {
  const ConvarService = useService(IConvarService);
  const LinkedIdentitiesService = useService(ILinkedIdentitiesService);

  React.useEffect(() => {
    LinkedIdentitiesService.addRockstar();
  }, []);

  if (ConvarService.getBoolean(KnownConvars.streamerMode)) {
    return (
      // eslint-disable-next-line react/jsx-no-useless-fragment
      <>{'<HIDDEN>'}</>
    );
  }

  const identityNodes: React.ReactNode[] = [];

  for (const identity of LinkedIdentitiesService.linkedIdentities.value) {
    if (!identity.username) {
      continue;
    }

    const username = escapeUsername(identity.username);
    const title = getLinkedIdentityTitle(identity);
    const icon = getProviderIcon(identity.provider);

    identityNodes.push(
      <Title key={identity.provider + identity.id} title={title}>
        <Flex gap="small">
          {icon}
          {username}
        </Flex>
      </Title>,
    );
  }

  return (
    <Flex gap="large">{identityNodes}</Flex>
  );
});

function getProviderIcon(provider: LinkedIdentityProvider): React.ReactNode {
  switch (provider) {
    case LinkedIdentityProvider.Steam: {
      return (
        <FaSteam />
      );
    }

    case LinkedIdentityProvider.Discord: {
      return (
        <FaDiscord />
      );
    }

    case LinkedIdentityProvider.XboxLive: {
      return (
        <FaXbox />
      );
    }

    case LinkedIdentityProvider.ROS: {
      return BrandIcon.ROS;
    }

    case LinkedIdentityProvider.Cfxre: {
      return (
        <FaRoad />
      );
    }
  }

  return null;
}

function getLinkedIdentityTitle(identity: ILinkedIdentity): React.ReactNode {
  switch (identity.provider) {
    case LinkedIdentityProvider.Discord: {
      return (
        <>Discord - {$L('#Settings_LinkedIdentity_AddedManually')}</>
      );
    }

    case LinkedIdentityProvider.Steam: {
      return (
        <>Steam - {$L('#Settings_LinkedIdentity_DetectedAutomatically')} 🎉</>
      );
    }

    case LinkedIdentityProvider.XboxLive: {
      return (
        <>Xbox Live - {$L('#Settings_LinkedIdentity_DetectedAutomatically')} 🎉</>
      );
    }

    case LinkedIdentityProvider.ROS: {
      return (
        <>Rockstar Online Services - {$L('#Settings_LinkedIdentity_DetectedAutomatically')} 🎉</>
      );
    }

    case LinkedIdentityProvider.Cfxre: {
      return 'Cfx.re';
    }
  }

  return identity.provider;
}

function escapeUsername(username: string): string {
  return username
    .replace(/&/g, '&amp;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;')
    .replace(/"/g, '&quot;')
    .replace(/'/g, '&#039;');
}
