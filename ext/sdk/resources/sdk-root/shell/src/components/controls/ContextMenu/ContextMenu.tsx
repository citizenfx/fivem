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
    const itemsNodes = items.map((item, index) => {
      if (item === ContextMenuItemSeparator) {
        return (
          <div key={index} className={s.separator}/>
        );
      } else {
        const handleItemMouseUp = () => {
          item.onClick();
          setCoords(null);
        };

        return (
          <div
            key={item.id}
            className={classnames(s.item, { [s.disabled]: item.disabled })}
            onMouseUp={handleItemMouseUp}
          >
            {item.icon || <BsAlt className={s.dummy} />}
            {item.text}
          </div>
        );
      }
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
