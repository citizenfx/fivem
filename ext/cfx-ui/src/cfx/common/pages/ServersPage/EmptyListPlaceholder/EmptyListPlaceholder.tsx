import React from "react";
import { ServerListConfigController } from "cfx/common/services/servers/lists/ServerListConfigController";
import { ServersListType } from "cfx/common/services/servers/lists/types";
import { Button } from "cfx/ui/Button/Button";
import { LinkButton } from "cfx/ui/Button/LinkButton";
import { Island } from "cfx/ui/Island/Island";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { Page } from "cfx/ui/Layout/Page/Page";
import { Text } from "cfx/ui/Text/Text";
import { observer } from "mobx-react-lite";
import { WiWindy } from "react-icons/wi";
import { SERVER_LIST_DESCRIPTORS } from "../ListTypeTabs";
import s from './EmptyListPlaceholder.module.scss';

export interface EmptyListPlaceholderProps {
  configController?: ServerListConfigController | undefined,
}

export const EmptyListPlaceholder = observer(function EmptyListPlaceholder(props: EmptyListPlaceholderProps) {
  const {
    configController,
  } = props;

  const possiblyDueToFilters = usePossiblyEmptyDueToFilters(configController);

  const resetFilters = React.useCallback(() => {
    configController?.reset();
  }, [configController]);

  return (
    <Page>
      <Island grow>
        <Flex fullWidth fullHeight vertical centered gap="xlarge">
          <WiWindy className={s.icon} />

          <Text size="xlarge" weight="bold" opacity="75">
            Looks pretty empty here
            {possiblyDueToFilters && ', try to relax or reset filters'}
          </Text>

          <Flex>
            <LinkButton
              to={SERVER_LIST_DESCRIPTORS[ServersListType.All].to}
              text={SERVER_LIST_DESCRIPTORS[ServersListType.All].title}
              size="large"
              theme={possiblyDueToFilters ? 'default' : 'primary'}
            />

            {possiblyDueToFilters && (
              <Button
                text="Reset filters"
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
