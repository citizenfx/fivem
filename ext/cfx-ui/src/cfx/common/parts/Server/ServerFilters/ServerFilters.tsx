import {
  Button,
  ButtonBar,
  Checkbox,
  ControlBox,
  Decorate,
  Dot,
  Icons,
  Interactive,
  Flex,
  Pad,
  VirtualScrollable,
  Separator,
  Text,
  Title,
  clsx,
} from '@cfx-dev/ui-components';
import { observer } from 'mobx-react-lite';
import React from 'react';
import {
  BsSortAlphaDown,
  BsSortAlphaDownAlt,
  BsSortDown,
  BsSortDownAlt,
  BsSortNumericDown,
  BsSortNumericDownAlt,
} from 'react-icons/bs';
import { FiFilter } from 'react-icons/fi';

import { useEventHandler } from 'cfx/common/services/analytics/analytics.service';
import { EventActionNames, ElementPlacements } from 'cfx/common/services/analytics/types';
import { useIntlService } from 'cfx/common/services/intl/intl.service';
import { $L } from 'cfx/common/services/intl/l10n';
import { ServerListConfigController } from 'cfx/common/services/servers/lists/ServerListConfigController';
import { ServerListSortDir, ServersListSortBy } from 'cfx/common/services/servers/lists/types';
import { useServersService } from 'cfx/common/services/servers/servers.service';
import { IAutocompleteIndexLocaleItem } from 'cfx/common/services/servers/source/types';
import { useUiService } from 'cfx/common/services/ui/ui.service';
import { Popover } from 'cfx/ui/Popover/Popover';

import { SearchInput } from './SearchInput/SearchInput';

import s from './ServerFilters.module.scss';

export interface ServerFiltersProps {
  config: ServerListConfigController;

  inputRef?: React.RefObject<HTMLElement>;
  onInputActive?(active: boolean): void;
  onInputKeyDown?(event: React.KeyboardEvent<HTMLInputElement>): void;
}

export const ServerFilters = observer(function ServerFilters(props: ServerFiltersProps) {
  const {
    config,
    inputRef,
    onInputActive,
    onInputKeyDown,
  } = props;

  const filtersDecorator = config.filteringByAnyTag || config.filteringByAnyLocale
    ? (
      <Dot />
      )
    : null;

  return (
    <>
      <SearchInput
        size="large"
        value={config.searchText}
        parsed={config.searchTextParsed}
        inputRef={inputRef}
        onChange={config.setSearchText}
        onActive={onInputActive}
        onKeyDown={onInputKeyDown}
      />

      <ButtonBar>
        <Popover at="top-right" popover={<FiltersPopover config={config} />}>
          {(active) => (
            <Decorate decorator={filtersDecorator}>
              <Button
                size="large"
                theme={(active && 'primary') || 'default-blurred'}
                icon={<FiFilter />}
                text={$L('#ServerList_Filter')}
              />
            </Decorate>
          )}
        </Popover>

        <Popover at="top-right" popover={<SortPopover config={config} />}>
          {(active) => (
            <Button
              theme={(active && 'primary') || 'default-blurred'}
              size="large"
              icon={iconsMap[config.sortBy][config.sortDir]}
            />
          )}
        </Popover>
      </ButtonBar>
    </>
  );
});

const FiltersPopover = observer(function FiltersPopover(props: ServerFiltersProps) {
  const {
    config,
  } = props;

  const UiService = useUiService();
  const ServersService = useServersService();
  const eventHandler = useEventHandler();

  const tagsList = ServersService.autocompleteIndex?.tag.sequence || [];
  const tags = ServersService.autocompleteIndex?.tag.items || {};

  const localesList = ServersService.autocompleteIndex?.locale.sequence || [];
  const locales = ServersService.autocompleteIndex?.locale.items || {};

  const hideEmptyTextKey = '#ServerList_HideEmpty';
  const hideFullTextKey = '#ServerList_HideFull';

  const handleFilterEvent = React.useCallback(
    (text: string) => {
      eventHandler({
        action: EventActionNames.FilterCTA,
        properties: {
          text,
          element_placement: ElementPlacements.Nav,
          link_url: '/',
        },
      });
    },
    [eventHandler],
  );

  const handleHideEmptyClick = React.useCallback(
    (value: boolean) => {
      handleFilterEvent(hideEmptyTextKey);
      config.setHideEmpty(value);
    },
    [handleFilterEvent, config],
  );

  const handleHideFullClick = React.useCallback(
    (value: boolean) => {
      handleFilterEvent(hideFullTextKey);
      config.setHideFull(value);
    },
    [handleFilterEvent, config],
  );

  return (
    <div className={clsx(s.popover, s['full-height'])}>
      <Flex fullHeight vertical>
        <Pad top left right>
          <Flex centered>
            <ControlBox size="large">
              <Text uppercase size="large" weight="bold" opacity="50">
                {$L('#ServerList_Filter')}
              </Text>
            </ControlBox>
          </Flex>
        </Pad>

        <Pad left right>
          <Flex stretch>
            <Checkbox
              size="large"
              label={$L(hideEmptyTextKey)}
              value={config.hideEmpty}
              onChange={handleHideEmptyClick}
            />
            <Checkbox size="large" label={$L(hideFullTextKey)} value={config.hideFull} onChange={handleHideFullClick} />
          </Flex>
        </Pad>

        <Flex fullHeight vertical stretch gap="none">
          <Flex vertical gap="none">
            <Separator />

            <Pad>
              <Flex repell centered>
                <ControlBox size="small">
                  <Text opacity="50">{$L('#ServerList_Locales')}</Text>
                </ControlBox>

                {config.filteringByAnyLocale && (
                  <Title title={$L('#ServerList_Filter_Reset')}>
                    <Button size="small" theme="transparent" icon={Icons.remove} onClick={config.clearLocalesFilter} />
                  </Title>
                )}
              </Flex>
            </Pad>

            <VirtualScrollable
              itemCount={localesList.length}
              itemHeight={UiService.quant * 6}
              renderItem={(index) => (
                <LocaleFaucet config={config} locale={locales[localesList[index]]} />
              )}
            />
          </Flex>

          <Flex vertical gap="none">
            <Separator />

            <Pad>
              <Flex repell centered>
                <ControlBox size="small">
                  <Text opacity="50">{$L('#ServerList_Tags')}</Text>
                </ControlBox>

                {config.filteringByAnyTag && (
                  <Title title="Reset">
                    <Button size="small" theme="transparent" icon={Icons.remove} onClick={config.clearTagsFilter} />
                  </Title>
                )}
              </Flex>
            </Pad>

            <VirtualScrollable
              itemCount={tagsList.length}
              itemHeight={UiService.quant * 6}
              renderItem={(index) => (
                <TagFaucet config={config} tag={tagsList[index]} count={tags[tagsList[index]].count} />
              )}
            />
          </Flex>
        </Flex>
      </Flex>
    </div>
  );
});

const SortPopover = observer(function SortPopover(props: ServerFiltersProps) {
  const {
    config,
  } = props;
  const eventHandler = useEventHandler();

  const byBoostTitleKey = '#Server_BoostPower_Title';
  const byNameTitleKey = '#ServerList_Name';
  const byPlayersTitleKey = '#ServerList_Players';

  const handleFilterEvent = React.useCallback(
    (text: string) => {
      eventHandler({
        action: EventActionNames.FilterCTA,
        properties: {
          text,
          element_placement: ElementPlacements.Nav,
          link_url: '/',
        },
      });
    },
    [eventHandler],
  );

  const handleByBoostClick = React.useCallback(() => {
    handleFilterEvent(byBoostTitleKey);
    config.setSortByBoosts();
  }, [handleFilterEvent, config]);

  const handleByNameClick = React.useCallback(() => {
    handleFilterEvent(byNameTitleKey);
    config.setSortByName();
  }, [handleFilterEvent, config]);

  const handleByPlayersClick = React.useCallback(() => {
    handleFilterEvent(byPlayersTitleKey);
    config.setSortByPlayers();
  }, [handleFilterEvent, config]);

  return (
    <div className={s.popover}>
      <Pad size="small" top bottom>
        <ListItem
          active={config.sortBy === ServersListSortBy.Boosts}
          label={$L(byBoostTitleKey)}
          value={Icons.serverBoost}
          onClick={handleByBoostClick}
        />
        <ListItem
          active={config.sortBy === ServersListSortBy.Name}
          label={$L(byNameTitleKey)}
          value={iconsMap[ServersListSortBy.Name][config.sortDir]}
          onClick={handleByNameClick}
        />
        <ListItem
          active={config.sortBy === ServersListSortBy.Players}
          label={$L(byPlayersTitleKey)}
          value={iconsMap[ServersListSortBy.Players][config.sortDir]}
          onClick={handleByPlayersClick}
        />
      </Pad>
    </div>
  );
});

const iconsMap = {
  [ServersListSortBy.Boosts]: {
    [ServerListSortDir.Desc]: <BsSortDownAlt />,
    [ServerListSortDir.Asc]: <BsSortDown />,
  },
  [ServersListSortBy.Name]: {
    [ServerListSortDir.Desc]: <BsSortAlphaDownAlt />,
    [ServerListSortDir.Asc]: <BsSortAlphaDown />,
  },
  [ServersListSortBy.Players]: {
    [ServerListSortDir.Desc]: <BsSortNumericDownAlt />,
    [ServerListSortDir.Asc]: <BsSortNumericDown />,
  },
};

interface LocaleFaucetProps {
  config: ServerListConfigController;
  locale: IAutocompleteIndexLocaleItem;
}
const LocaleFaucet = observer(function LocaleFaucet(props: LocaleFaucetProps) {
  const {
    config,
    locale,
  } = props;

  const IntlService = useIntlService();
  const eventHandler = useEventHandler();

  const handleClick = React.useCallback(() => {
    eventHandler({
      action: EventActionNames.FilterCTA,
      properties: {
        text: locale.locale,
        element_placement: ElementPlacements.Nav,
        link_url: '/',
      },
    });

    config.toggleLocale(locale.locale);
  }, [eventHandler, locale, config]);

  return (
    <Faucet
      active={config.getLocale(locale.locale)}
      left={(
        <Flex>
          <span className={`fi fi-${locale.country.toLowerCase()}`} />

          <span>{IntlService.defaultDisplayNames.of(locale.locale)}</span>
        </Flex>
      )}
      right={locale.count}
      onClick={handleClick}
    />
  );
});

interface TagFaucetProps {
  config: ServerListConfigController;
  tag: string;
  count: number;
}
const TagFaucet = observer(function TagFaucet(props: TagFaucetProps) {
  const {
    config,
    tag,
    count,
  } = props;
  const eventHandler = useEventHandler();

  const handleClick = React.useCallback(() => {
    eventHandler({
      action: EventActionNames.FilterCTA,
      properties: {
        text: tag,
        element_placement: ElementPlacements.Nav,
        link_url: '/',
      },
    });

    config.toggleTag(tag);
  }, [eventHandler, tag, config]);

  return (
    <Faucet active={config.getTag(tag)} left={tag} right={count} onClick={handleClick} />
  );
});

interface FaucetProps {
  active?: boolean | undefined;
  left: React.ReactNode;
  right: React.ReactNode;
  onClick(): void;
}
function Faucet(props: FaucetProps) {
  const {
    active,
    left,
    right,
    onClick,
  } = props;

  const faucetClassName = clsx(s.faucet, {
    [s.inclusive]: active === true,
    [s.exclusive]: active === false,
  });

  return (
    <Interactive className={faucetClassName} onClick={onClick}>
      <div className={s.left}>{left}</div>
      <div className={s.right}>{right}</div>
    </Interactive>
  );
}

interface ListItemProps {
  active?: boolean;
  label: React.ReactNode;
  value: React.ReactNode;
  onClick(): void;
}
function ListItem({
  active,
  onClick,
  label,
  value,
}: ListItemProps) {
  const itemClassName = clsx(s.item, {
    [s.active]: active,
  });

  return (
    <Interactive className={itemClassName} onClick={onClick}>
      <div className={s.label}>{label}</div>
      <div className={s.value}>{value}</div>
    </Interactive>
  );
}
