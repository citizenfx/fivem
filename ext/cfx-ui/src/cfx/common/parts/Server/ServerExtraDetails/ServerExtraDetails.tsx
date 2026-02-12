import {
  Icons,
  Flex,
  Text,
  Title,
  ui,
  Linkify,
  Separator,
  Icon,
} from '@cfx-dev/ui-components';
import { observer } from 'mobx-react-lite';
import React from 'react';

import { $L } from 'cfx/common/services/intl/l10n';
import { IServerView } from 'cfx/common/services/servers/types';

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

  if (varNodes.length === 0) {
    return null;
  }

  return (
    <Flex vertical>
      <Flex repell centered>
        <Flex centered gap="small">
          <Icon size="normal" opacity="50">
            {Icons.visibility}
          </Icon>

          <Text uppercase size="normal" opacity="50" weight="bold">
            {$L('#ServerDetail_Details')}
          </Text>
        </Flex>

        <Separator thin />
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
