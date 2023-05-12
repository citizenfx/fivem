import React from "react";
import { observer } from "mobx-react-lite";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { Text } from "cfx/ui/Text/Text";
import { Loaf } from "cfx/ui/Loaf/Loaf";
import { Button } from "cfx/ui/Button/Button";
import { Box } from "cfx/ui/Layout/Box/Box";
import { Flyout } from "cfx/ui/Flyout/Flyout";
import { Icons } from "cfx/ui/Icons";
import { FlexRestricter } from "cfx/ui/Layout/Flex/FlexRestricter";
import { Scrollable } from "cfx/ui/Layout/Scrollable/Scrollable";
import { Input } from "cfx/ui/Input/Input";
import { Separator } from "cfx/ui/Separator/Separator";
import s from './LongListSideSection.module.scss';

export interface LongListSideSectionProps<T> {
  items: T[],
  renderPreviewItem: (item: T) => React.ReactNode,
  renderFullItem?: (item: T) => React.ReactNode,
  itemMatchesFilter?: (filter: string, item: T) => boolean,

  maxPreviewItems?: number,

  icon?: React.ReactNode,
  title: React.ReactNode,
  subtitle?: React.ReactNode,

  seeAllTitle?: React.ReactNode,
}

export const LongListSideSection = observer(function LongListSideSection<T>(props: LongListSideSectionProps<T>) {
  const {
    items,
    renderPreviewItem,
    renderFullItem = renderPreviewItem,
    itemMatchesFilter,
    maxPreviewItems = 18,

    icon,
    title,
    subtitle = items.length || 0,

    seeAllTitle = 'See all',
  } = props;

  const [fullListActive, setFullListActive] = React.useState(false);
  const handleOpenFullList = React.useCallback(() => setFullListActive(true), []);
  const handleCloseFullList = React.useCallback(() => setFullListActive(false), []);

  const containerRef = React.useRef<HTMLDivElement>(null);
  const previewRef = React.useRef<HTMLDivElement>(null);

  const previewFits = usePreviewFits(containerRef, previewRef);

  const previewNodes = React.useMemo(() => items.slice(0, maxPreviewItems).map((item, i) => (
    <Loaf key={i} size="small">
      {renderPreviewItem(item)}
    </Loaf>
  )), [items, maxPreviewItems]);

  return (
    <>
      {fullListActive && (
        <FullListFlyout
          onClose={handleCloseFullList}
          icon={icon}
          title={title}
          subtitle={subtitle}
          items={items}
          renderFullItem={renderFullItem}
          itemMatchesFilter={itemMatchesFilter}
        />
      )}

      <Flex vertical>
        <Flex repell centered>
          <Flex centered gap="small">
            <Text size="small" opacity="75" >
              {icon}
            </Text>

            <Text uppercase size="small" opacity="75">
              {title}
            </Text>
          </Flex>

          <Separator thin />

          <Box noShrink>
            {subtitle}
          </Box>
        </Flex>

        {!!previewNodes.length && (
          <Flex fullWidth gap="small" ref={containerRef}>
            <div className={s.preview} ref={previewRef}>
              {previewNodes}
            </div>

            {!previewFits && (
              <Box noShrink>
                <Button
                  size="small"
                  text={seeAllTitle}
                  onClick={handleOpenFullList}
                />
              </Box>
            )}
          </Flex>
        )}
      </Flex>
    </>
  );
});

function usePreviewFits(containerRef: React.RefObject<HTMLDivElement>, previewRef: React.RefObject<HTMLDivElement>): boolean {
  const [fits, setFits] = React.useState(false);

  React.useLayoutEffect(() => {
    if (!containerRef.current || !previewRef.current) {
      return;
    }

    const containerWidth = containerRef.current.getBoundingClientRect().width;
    const previewWidth = previewRef.current.scrollWidth;

    setFits(previewWidth <= containerWidth);
  }, []);

  return fits;
}

interface FullListFlyoutProps<T> {
  onClose(): void,

  icon?: React.ReactNode,
  title: React.ReactNode,
  subtitle: React.ReactNode,
  items: T[],
  renderFullItem(item: T): React.ReactNode,

  itemMatchesFilter: LongListSideSectionProps<T>['itemMatchesFilter'],
};

function FullListFlyout<T>(props: FullListFlyoutProps<T>) {
  const {
    onClose,

    icon,
    title,
    subtitle,

    items,
    renderFullItem,
    itemMatchesFilter,
  } = props;

  const [filter, setFilter] = React.useState('');

  const itemsToRender = (filter && itemMatchesFilter)
    ? items.filter((item) => itemMatchesFilter(filter, item))
    : items;

  return (
    <Flyout onClose={onClose} size="xsmall">
      <Flex vertical fullHeight gap="large">
        <Flex centered repell gap="large">
          <Flex centered gap="large">
            <Flex centered>
              <Text size="large" opacity="75" >
                {icon}
              </Text>

              <Text uppercase size="large" opacity="75">
                {title}
              </Text>
            </Flex>

            {subtitle}
          </Flex>

          <Box noShrink>
            <Button
              size="large"
              icon={Icons.exit}
              onClick={onClose}
            />
          </Box>
        </Flex>

        {Boolean(itemMatchesFilter) && (
          <Input
            autofocus
            type="search"
            value={filter}
            onChange={setFilter}
            placeholder="Filter"
          />
        )}

        <FlexRestricter vertical>
          <Scrollable>
            <Flex fullWidth wrap>
              {itemsToRender.map((item, i) => (
                <Loaf
                  key={i}
                  bright
                  className={s.item}
                >
                  {renderFullItem(item)}
                </Loaf>
              ))}
            </Flex>
          </Scrollable>
        </FlexRestricter>
      </Flex>
    </Flyout>
  );
}
