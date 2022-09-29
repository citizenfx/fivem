import React from "react";
import { ServerListConfigController } from "cfx/common/services/servers/lists/ServerListConfigController";
import { ServerListSortDir, ServersListSortBy } from "cfx/common/services/servers/lists/types";
import { Button } from "cfx/ui/Button/Button";
import { Popover } from "cfx/ui/Popover/Popover";
import { clsx } from "cfx/utils/clsx";
import { observer } from "mobx-react-lite";
import { BsSortAlphaDown, BsSortAlphaDownAlt, BsSortDown, BsSortDownAlt, BsSortNumericDown, BsSortNumericDownAlt } from "react-icons/bs";
import { FiFilter } from 'react-icons/fi';
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { $L } from "cfx/common/services/intl/l10n";
import { Checkbox } from "cfx/ui/Checkbox/Checkbox";
import { Pad } from "cfx/ui/Layout/Pad/Pad";
import { Text } from "cfx/ui/Text/Text";
import { Separator } from "cfx/ui/Separator/Separator";
import { VirtualScrollable } from "cfx/ui/Layout/Scrollable/VirtualScrollable";
import { Title } from "cfx/ui/Title/Title";
import { ControlBox } from "cfx/ui/ControlBox/ControlBox";
import { IAutocompleteIndexLocaleItem } from "cfx/common/services/servers/source/types";
import { Decorate } from "cfx/ui/Decorate/Decorate";
import { ButtonBar } from "cfx/ui/Button/ButtonBar";
import { Dot } from "cfx/ui/Dot/Dot";
import { Icons } from "cfx/ui/Icons";
import { useServersService } from "cfx/common/services/servers/servers.service";
import { useIntlService } from "cfx/common/services/intl/intl.service";
import { useUiService } from "cfx/common/services/ui/ui.service";
import { SearchInput } from "./SearchInput/SearchInput";
import s from './ServerFilters.module.scss';

export interface ServerFiltersProps {
  config: ServerListConfigController,

  inputRef?: React.RefObject<HTMLElement>,
  onInputActive?(active: boolean): void,
  onInputKeyDown?(event: React.KeyboardEvent<HTMLInputElement>): void,
}

export const ServerFilters = observer(function ServerFilters(props: ServerFiltersProps) {
  const {
    config,
    inputRef,
    onInputActive,
    onInputKeyDown,
  } = props;

  const filtersDecorator = (config.filteringByAnyTag || config.filteringByAnyLocale)
    ? <Dot />
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
          {(ref, active) => (
            <Decorate
              ref={ref as any}
              decorator={filtersDecorator}
            >
              <Button
                size="large"
                theme={active && 'primary' || 'default-blurred'}
                icon={<FiFilter />}
                text="Filters"
              />
            </Decorate>
          )}
        </Popover>

        <Popover at="top-right" popover={<SortPopover config={config} />}>
          {(ref, active) => (
            <Button
              ref={ref as any}
              theme={active && 'primary' || 'default-blurred'}
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
  const { config } = props;

  const UiService = useUiService();
  const ServersService = useServersService();

  const tagsList = ServersService.autocompleteIndex?.tag.sequence || [];
  const tags = ServersService.autocompleteIndex?.tag.items || {};

  const localesList = ServersService.autocompleteIndex?.locale.sequence || [];
  const locales = ServersService.autocompleteIndex?.locale.items || {};

  return (
    <div className={clsx(s.popover, s['full-height'])}>
      <Flex fullHeight vertical>
        <Pad top left right>
          <Flex centered>
            <ControlBox size="large">
              <Text size="large" weight="bold" opacity="50">
                FILTERS
              </Text>
            </ControlBox>
          </Flex>
        </Pad>

        <Pad left right>
          <Flex stretch>
            <Checkbox
              size="large"
              label={$L('#ServerList_HideEmpty')}
              value={config.hideEmpty}
              onChange={config.setHideEmpty}
            />
            <Checkbox
              size="large"
              label={$L('#ServerList_HideFull')}
              value={config.hideFull}
              onChange={config.setHideFull}
            />
          </Flex>
        </Pad>

        <Flex fullHeight vertical stretch gap="none">
          <Flex vertical gap="none">
            <Separator />

            <Pad>
              <Flex repell centered>
                <ControlBox size="small">
                  <Text opacity="50">
                    Locales
                  </Text>
                </ControlBox>

                {config.filteringByAnyLocale && (
                  <Title title="Reset">
                    <Button
                      size="small"
                      theme="transparent"
                      icon={Icons.remove}
                      onClick={config.clearLocalesFilter}
                    />
                  </Title>
                )}
              </Flex>
            </Pad>

            <VirtualScrollable
              itemCount={localesList.length}
              itemHeight={UiService.quant * 6}
              renderItem={(index) => (
                <LocaleFaucet
                  config={config}
                  locale={locales[localesList[index]]}
                />
              )}
            />
          </Flex>

          <Flex vertical gap="none">
            <Separator />

            <Pad>
              <Flex repell centered>
                <ControlBox size="small">
                  <Text opacity="50">
                    Tags
                  </Text>
                </ControlBox>

                {config.filteringByAnyTag && (
                  <Title title="Reset">
                    <Button
                      size="small"
                      theme="transparent"
                      icon={Icons.remove}
                      onClick={config.clearTagsFilter}
                    />
                  </Title>
                )}
              </Flex>
            </Pad>

            <VirtualScrollable
              itemCount={tagsList.length}
              itemHeight={UiService.quant * 6}
              renderItem={(index) => (
                <TagFaucet
                  config={config}
                  tag={tagsList[index]}
                  count={tags[tagsList[index]].count}
                />
              )}
            />
          </Flex>
        </Flex>
      </Flex>
    </div>
  );
});

const SortPopover = observer(function SortPopover(props: ServerFiltersProps) {
  const { config } = props;

  return (
    <div className={s.popover}>
      <Pad size="small" top bottom>
        <ListItem
          active={config.sortBy === ServersListSortBy.Boosts}
          label="BOOST Power"
          value={Icons.serverBoost}
          onClick={config.setSortByBoosts}
        />
        <ListItem
          active={config.sortBy === ServersListSortBy.Name}
          label="Server name"
          value={iconsMap[ServersListSortBy.Name][config.sortDir]}
          onClick={config.setSortByName}
        />
        <ListItem
          active={config.sortBy === ServersListSortBy.Players}
          label="Players"
          value={iconsMap[ServersListSortBy.Players][config.sortDir]}
          onClick={config.setSortByPlayers}
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
  config: ServerListConfigController,
  locale: IAutocompleteIndexLocaleItem,
}
const LocaleFaucet = observer(function LocaleFaucet(props: LocaleFaucetProps) {
  const { config, locale } = props;

  const IntlService = useIntlService();

  return (
    <Faucet
      active={config.getLocale(locale.locale)}
      left={(
        <Flex>
          <span className={`fi fi-${locale.country.toLowerCase()}`} />

          <span>
            {IntlService.defaultDisplayNames.of(locale.locale)}
          </span>
        </Flex>
      )}
      right={locale.count}
      onClick={() => config.toggleLocale(locale.locale)}
    />
  );
});

interface TagFaucetProps {
  config: ServerListConfigController,
  tag: string,
  count: number,
}
const TagFaucet = observer(function TagFaucet(props: TagFaucetProps) {
  const { config, tag, count } = props;

  return (
    <Faucet
      active={config.getTag(tag)}
      left={tag}
      right={count}
      onClick={() => config.toggleTag(tag)}
    />
  );
});

interface FaucetProps {
  active?: boolean | undefined,
  left: React.ReactNode,
  right: React.ReactNode,
  onClick(): void,
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
    <div className={faucetClassName} onClick={onClick}>
      <div className={s.left}>
        {left}
      </div>
      <div className={s.right}>
        {right}
      </div>
    </div>
  );
}

interface ListItemProps {
  active?: boolean,
  label: React.ReactNode,
  value: React.ReactNode,
  onClick(): void,
}
function ListItem(props: ListItemProps) {
  const itemClassName = clsx(s.item, {
    [s.active]: props.active,
  });

  return (
    <div className={itemClassName} onClick={props.onClick}>
      <div className={s.label}>
        {props.label}
      </div>
      <div className={s.value}>
        {props.value}
      </div>
    </div>
  );
}
