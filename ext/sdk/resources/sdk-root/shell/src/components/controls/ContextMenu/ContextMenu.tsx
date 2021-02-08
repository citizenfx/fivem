import React, { MouseEvent } from 'react';
import ReactDOM from 'react-dom';
import classnames from 'classnames';
import s from './ContextMenu.module.scss';
import { BsAlt } from 'react-icons/bs';


const noop = () => {};


export const ContextMenuItemSeparator = Symbol('context-menu-separator');

export interface ContextMenuItem {
  id: string,
  text: React.ReactNode,
  icon?: React.ReactNode,
  onClick: () => void,
  disabled?: boolean,
}

export type ContextMenuItemsCollection = (ContextMenuItem | typeof ContextMenuItemSeparator)[];

export interface ContextMenuProps {
  children: React.ReactNode,
  items: ContextMenuItemsCollection,
  disabled?: boolean,
  title?: string,
  className?: string,
  activeClassName?: string,
  onClick?: (openMenu?: () => void) => void,
  getCoords?: (node: HTMLDivElement) => { top: number, left: number },
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
    getCoords,
  } = props;

  const [coords, setCoords] = React.useState<Coords | null>(null);
  const [outlet, setOutlet] = React.useState<HTMLElement | null>(null);

  const menuRef = React.useRef<HTMLDivElement | null>(null);

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
  const handleCloseMenu = React.useCallback(() => {
    setCoords(null);
  }, []);

  const handleClick = React.useCallback((event: MouseEvent) => {
    if (!coords) {
      onClick(() => handleOpenMenu(event));
    }
  }, [onClick, coords, handleOpenMenu]);

  let menu: React.ReactPortal | null = null;
  if (coords && outlet) {
    const itemsNodes = items.map((item, index) => {
      if (item === ContextMenuItemSeparator) {
        return (
          <div key={index} className={s.separator}/>
        );
      } else {
        return (
          <div
            key={item.id}
            className={classnames(s.item, { [s.disabled]: item.disabled })}
            onClick={item.onClick}
          >
            {item.icon || <BsAlt className={s.dummy} />}
            {item.text}
          </div>
        );
      }
    });

    menu = ReactDOM.createPortal(
      <div className={s.backdrop} onClick={handleCloseMenu}>
        <div ref={menuRef} className={s.menu} style={{ top: coords.top + 'px', left: coords.left + 'px' }}>
          {itemsNodes}
        </div>
      </div>,
      outlet,
    );
  }

  return (
    <div
      ref={ref}
      title={title}
      className={classnames(className, { [activeClassName]: !!coords })}
      onContextMenu={handleOpenMenu}
      onClick={handleClick}
    >
      {children}
      {menu}
    </div>
  );
});
