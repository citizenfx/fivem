import {
  Icons,
  Flex,
  Text,
  Title,
  ui,
  Linkify,
} from '@cfx-dev/ui-components';
import { observer } from 'mobx-react-lite';
import React from 'react';

import { ElementPlacements } from 'cfx/common/services/analytics/types';
import { IServerView } from 'cfx/common/services/servers/types';

import { ServerCoreLoafs } from '../ServerCoreLoafs/ServerCoreLoafs';

export const ServerExtraDetails = observer(function ServerExtraDetails({
  server,
}: { server: IServerView }) {
  const varNodes: React.ReactNode[] = [];

  if (server.variables) {
    for (const [name, value] of Object.entries(server.variables)) {
      varNodes.push(
        <Flex key={name + value}>
          <Text opacity="75" className={ui.cls.flexNoShrink}>
            {name}:
          </Text>
          <span>
            <Linkify text={value} replacer={linkReplacer} />
          </span>
        </Flex>,
      );
    }
  }

  return (
    <Flex vertical gap="large">
      <Flex wrap>
        <ServerCoreLoafs server={server} elementPlacement={ElementPlacements.ServerExtraDetails} />
      </Flex>

      {Boolean(varNodes.length) && (
        <Flex vertical>{varNodes}</Flex>
      )}
    </Flex>
  );
});

const linkReplacer: React.ComponentProps<typeof Linkify>['replacer'] = (key, text, url) => (
  <Title
    key={key}
    title={(
      <>
        {Icons.externalLink} {url}
      </>
    )}
  >
    <a href={url}>{text.replace('http://', '').replace('https://', '')}</a>
  </Title>
);
