import React, { MouseEvent } from 'react';
import ReactDOM from 'react-dom';
import classnames from 'classnames';
import s from './ContextMenu.module.scss';
import { BsAlt } from 'react-icons/bs';


const noop = () => {};

function ifVisible(item: IContextMenuItem): boolean {
  if (item === ContextMenuItemSeparator) {
    return true;
  }

  if (item.visible) {
    return item.visible();
  }

  return true;
}

export const ContextMenuItemSeparator = Symbol('context-menu-separator');

export interface ContextMenuItem {
  id: string,
  text: React.ReactNode,
  icon?: React.ReactNode,
  onClick: () => void,
  disabled?: boolean,
  visible?: () => boolean,
}

export type SimpleContextMenuItem =
  | ContextMenuItem
  | typeof ContextMenuItemSeparator;

export interface ContextMenuItemsGroup {
  visible?: () => boolean,
  items: SimpleContextMenuItem[],
}

type IContextMenuItem =
  | SimpleContextMenuItem
  | ContextMenuItemsGroup;

export type ContextMenuItemsCollection = Array<IContextMenuItem>;

export interface ContextMenuProps {
  children: React.ReactNode,
  items: ContextMenuItemsCollection,
  disabled?: boolean,
  title?: string,
  className?: string,
  activeClassName?: string,
  onClick?: (openMenu?: () => void) => void,
  getCoords?: (node: HTMLDivElement) => { top: number, left: number },
  elementProps?: object | null,

  onDoubleClick?(): void,
  onMouseOver?(): void,
  onMouseOut?(): void,
}

interface Coords {
  top: number,
  left: number,
}

export const ContextMenu = React.forwardRef(function ContextMenu(props: ContextMenuProps, ref: any) {
  const {
    children,
    items,
    title = '',
    className = '',
    activeClassName = '',
    onClick = noop,
    onDoubleClick = noop,
    elementProps = null,
    getCoords,
    onMouseOver = noop,
    onMouseOut = noop,
  } = props;

  const [coords, setCoords] = React.useState<Coords | null>(null);
  const [outlet, setOutlet] = React.useState<HTMLElement | null>(null);

  const menuRef = React.useRef<HTMLDivElement | null>(null);
  const backdropRef = React.useRef<HTMLDivElement | null>(null);

  React.useLayoutEffect(() => {
    setOutlet(document.getElementById('context-menu-outlet'));
  }, []);

  React.useLayoutEffect(() => {
    if (coords && menuRef.current) {
      const rect = menuRef.current.getBoundingClientRect();
      const viewportHeight = window.visualViewport.height;

      const viewportOverlap = viewportHeight - rect.bottom;

      if (viewportOverlap < 0) {
        setCoords({
          top: coords.top + viewportOverlap,
          left: coords.left,
        });
      }
    }
  }, [coords, setCoords, menuRef]);

  const renderItem = React.useCallback((item: SimpleContextMenuItem, index: number) => {
    if (item === ContextMenuItemSeparator) {
      return (
        <div key={index} className={s.separator}/>
      );
    }

    const handleItemMouseup = () => {
      item.onClick();
      setCoords(null);
    };

    return (
      <div
        key={item.id}
        className={classnames(s.item, { [s.disabled]: item.disabled })}
        onMouseUp={handleItemMouseup}
      >
        {item.icon || <BsAlt className={s.dummy} />}
        {item.text}
      </div>
    );
  }, []);

  const handleOpenMenu = React.useCallback((event: MouseEvent) => {
    event.preventDefault();
    event.stopPropagation();

    let coords;

    if (getCoords) {
      coords = getCoords(event.target as HTMLDivElement);
    } else {
      coords = {
        top: event.pageY,
        left: event.pageX,
      };
    }

    setCoords(coords);

    return false;
  }, [getCoords]);

  const handleBackdropMouseDown = React.useCallback((event: MouseEvent) => {
    if (backdropRef.current === event.target) {
      setCoords(null);
    }
  }, []);

  const handleRootClick = React.useCallback((event: MouseEvent) => {
    if (!coords) {
      onClick(() => handleOpenMenu(event));
    }
  }, [onClick, coords, handleOpenMenu]);

  let menu: React.ReactPortal | null = null;
  if (coords && outlet) {
    const visibleItems = items.filter(ifVisible);

    const itemsNodes = visibleItems.map((item, index) => {
      if (typeof item === 'object' && 'items' in item) {
        const groupItems = item.items.filter(ifVisible);
        if (!groupItems.length) {
          return null;
        }

        const groupItemNodes = groupItems.map(renderItem);

        return (
          <React.Fragment key={index}>
            {groupItemNodes}
            {(index < (visibleItems.length - 1)) && (
              <div className={s.separator} />
            )}
          </React.Fragment>
        );
      }

      return renderItem(item, index);
    });

    menu = ReactDOM.createPortal(
      <div ref={backdropRef} className={s.backdrop} onMouseDown={handleBackdropMouseDown}>
        <div ref={menuRef} className={s.menu} style={{ top: coords.top + 'px', left: coords.left + 'px' }}>
          {itemsNodes}
        </div>
      </div>,
      outlet,
    );
  }

  return (
    <div
      {...elementProps}
      ref={ref}
      title={title}
      className={classnames(className, { [activeClassName]: !!coords })}
      onContextMenu={handleOpenMenu}
      onClick={handleRootClick}
      onDoubleClick={onDoubleClick}
      onMouseOver={onMouseOver}
      onMouseOut={onMouseOut}
    >
      {children}
      {menu}
    </div>
  );
});
