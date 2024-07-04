import {
  Button,
  Island,
  Flex,
  Page,
  Text,
} from '@cfx-dev/ui-components';
import { observer } from 'mobx-react-lite';
import React from 'react';
import { WiWindy } from 'react-icons/wi';

import { $L } from 'cfx/common/services/intl/l10n';
import { ServerListConfigController } from 'cfx/common/services/servers/lists/ServerListConfigController';
import { ServersListType } from 'cfx/common/services/servers/lists/types';
import { LinkButton } from 'cfx/ui/Button/LinkButton';

import { SERVER_LIST_DESCRIPTORS } from '../ListTypeTabs';

import s from './EmptyListPlaceholder.module.scss';

export interface EmptyListPlaceholderProps {
  configController?: ServerListConfigController | undefined;
}

export const EmptyListPlaceholder = observer(function EmptyListPlaceholder(props: EmptyListPlaceholderProps) {
  const {
    configController,
  } = props;

  const possiblyEmptyDueToFilters = usePossiblyEmptyDueToFilters(configController);

  const resetFilters = React.useCallback(() => {
    configController?.reset();
  }, [configController]);

  return (
    <Page>
      <Island grow>
        <Flex fullWidth fullHeight vertical centered gap="xlarge">
          <WiWindy className={s.icon} />

          <Text size="xlarge" weight="bold" opacity="75">
            {$L('#ServerList_EmptyPlaceholder')}
            {possiblyEmptyDueToFilters && $L('#ServerList_EmptyPlaceholder_RelaxFilters')}
          </Text>

          <Flex>
            <LinkButton
              to={SERVER_LIST_DESCRIPTORS[ServersListType.All].to}
              text={$L(SERVER_LIST_DESCRIPTORS[ServersListType.All].titleKey)}
              size="large"
              theme={possiblyEmptyDueToFilters
                ? 'default'
                : 'primary'}
            />

            {possiblyEmptyDueToFilters && (
              <Button
                text={$L('#ServerList_EmptyPlaceholder_ResetFilters')}
                size="large"
                theme="primary"
                onClick={resetFilters}
              />
            )}
          </Flex>
        </Flex>
      </Island>
    </Page>
  );
});

function usePossiblyEmptyDueToFilters(configController?: ServerListConfigController): boolean {
  if (!configController) {
    return false;
  }

  const config = configController.get();

  if (Object.keys(config.tags).length) {
    return true;
  }

  if (Object.keys(config.locales).length) {
    return true;
  }

  if (config.hideEmpty) {
    return true;
  }

  if (config.hideFull) {
    return true;
  }

  if (config.searchTextParsed.length) {
    return true;
  }

  return false;
}
