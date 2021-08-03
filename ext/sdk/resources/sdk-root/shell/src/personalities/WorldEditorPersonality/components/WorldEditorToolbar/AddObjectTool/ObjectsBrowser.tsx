import React from 'react';
import { observer } from 'mobx-react-lite';
import AutoSizer from 'react-virtualized-auto-sizer';
import { useDebouncedCallback } from 'utils/hooks';
import { Input } from 'components/controls/Input/Input';
import { FixedSizeList } from 'react-window';
import { WEState } from 'personalities/WorldEditorPersonality/store/WEState';
import { BsArrowClockwise } from 'react-icons/bs';
import { GameState } from 'store/GameState';
import { WETool, WEToolbarState } from '../../../store/WEToolbarState';
import { ArchetypesState } from 'personalities/WorldEditorPersonality/store/ArchetypesState';
import { WORLD_EDITOR_MAP_NO_GROUP } from 'backend/world-editor/world-editor-constants';
import { Title } from 'components/controls/Title/Title';
import s from './AddObjectTool.module.scss';

interface ObjectItemProps {
  name: string,

  active: boolean,
  style: React.CSSProperties,

  spawn(name: string): void,
  activate(): void,
}

const ObjectItem = React.memo(function ObjectItem({ active, name, style, spawn, activate }: ObjectItemProps) {
  const className = s.item + ' ' + (active && s.active);
  const handleClick = () => spawn(name);

  return (
    <div
      style={style}
      className={className}
      onClick={handleClick}
      onMouseEnter={activate}
    >
      {name}
    </div>
  );
});

export const ObjectsBrowser = observer(function ObjectsBrowserDropdown() {
  const listRef = React.useRef<FixedSizeList>();

  const [filter, setFilter] = React.useState('');
  const [filtered, setFiltered] = React.useState<string[]>([]);
  const [activeIndex, setActiveIndex] = React.useState(0);

  const performFiltering = useDebouncedCallback(async (newFilter: string) => {
    setFiltered((await ArchetypesState.search(newFilter)) || []);
  }, 50, []);

  const activeSet: string[] = filter.length ? filtered : ArchetypesState.archetypes;

  // Change preview item
  React.useEffect(() => {
    const name = activeSet[activeIndex];

    sendGameClientEvent('setPreviewObjectName', name || '');
  }, [activeSet, activeIndex]);

  // Clear preview item on unmount
  React.useEffect(() => () => sendGameClientEvent('setPreviewObjectName', ''), []);

  const handleSetFilter = React.useCallback((newFilter: string) => {
    const normalizedFilter = newFilter.trim();

    setFilter(normalizedFilter);
    setActiveIndex(0);

    if (!normalizedFilter) {
      setFiltered([]);
    } else {
      performFiltering(normalizedFilter);
    }
  }, []);

  const handleFilterKeyDown = React.useCallback((event: React.KeyboardEvent<HTMLInputElement>) => {
    const isUp = event.key === 'ArrowUp';
    const isDown = event.key === 'ArrowDown';
    const isEsc = event.key === 'Escape';
    const isHome = event.key === 'Home';
    const isEnd = event.key === 'End';
    const isPageUp = event.key === 'PageUp';
    const isPageDown = event.key === 'PageDown';

    if (isUp || isDown || isEsc || isHome || isEnd || isPageUp || isPageDown) {
      event.preventDefault();
      event.stopPropagation();

      if (isEsc) {
        WEToolbarState.closeTool(WETool.AddObject);

        return true;
      }

      let newIndex = activeIndex;
      const maxIndex = activeSet.length - 1;

      if (isUp || isDown) {
        const dir = isUp ? -1 : +1;

        newIndex = activeIndex + dir;

        if (newIndex < 0) {
          newIndex = maxIndex;
        } else if (newIndex > maxIndex) {
          newIndex = 0;
        }
      } else if (isHome) {
        newIndex = 0;
      } else if (isEnd) {
        newIndex = maxIndex;
      } else if (isPageUp) {
        newIndex -= 20;
        if (newIndex < 0) {
          newIndex = 0;
        }
      } else if (isPageDown) {
        newIndex += 20;
        if (newIndex > maxIndex) {
          newIndex = maxIndex;
        }
      }

      if (newIndex !== activeIndex) {
        setActiveIndex(newIndex);
        listRef.current.scrollToItem(newIndex);
      }

      return true;
    }
  }, [activeIndex, activeSet]);

  const handleCreateAddition = React.useCallback((objectName: string) => {
    if (objectName) {
      WEState.map.createAddition(objectName, WORLD_EDITOR_MAP_NO_GROUP);
    }

    WEToolbarState.closeTool(WETool.AddObject);
  }, []);

  const handleFilterSubmit = React.useCallback(() => {
    const name = activeSet[activeIndex];

    handleCreateAddition(name);
  }, [activeSet, activeIndex]);

  return (
    <div className={s.root}>
      <div className={s.controls}>
        <Input
          autofocus
          value={filter}
          className={s.filter}
          placeholder="Search objects"
          onChange={handleSetFilter}
          onKeyDown={handleFilterKeyDown}
          onSubmit={handleFilterSubmit}
        />

        <Title animated={false} delay={0} fixedOn="bottom" title="Refresh objects list">
          {(ref) => (
            <button
              ref={ref}
              className={s.refresh}
              onClick={() => GameState.refreshArchetypesCollection()}
            >
              <BsArrowClockwise />
            </button>
          )}
        </Title>
      </div>

      <div className={s.list}>
        <AutoSizer>
          {({ width, height }) => (
            <FixedSizeList
              ref={listRef}
              width={width}
              height={height}
              itemCount={activeSet.length}
              itemSize={28}
            >
              {({ index, style }) => {
                const name = activeSet[index];
                const activate = () => setActiveIndex(index);

                return (
                  <ObjectItem
                    key={index}
                    active={activeIndex === index}
                    name={name}
                    style={style}
                    spawn={handleCreateAddition}
                    activate={activate}
                  />
                );
              }}
            </FixedSizeList>
          )}
        </AutoSizer>
      </div>
    </div>
  );
});
