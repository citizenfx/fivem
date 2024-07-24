import {
  Indicator,
  RichInput,
  ui,
  noop,
  clsx,
  useOutlet,
  mergeRefs,
  splitByIndices,
  TITLE_OUTLET_ID,
} from '@cfx-dev/ui-components';
import { observer } from 'mobx-react-lite';
import React from 'react';

import { ISearchTerm } from 'cfx/base/searchTermsParser';
import { useL10n } from 'cfx/common/services/intl/l10n';
import { useWindowResize } from 'cfx/utils/hooks';

import { Cheatsheet } from './Cheatsheet/Cheatsheet';
import { SearchInputController, SuggestionState, useSearchInputController } from './SearchInputController';

import s from './SearchInput.module.scss';

type RichInputProps = React.ComponentProps<typeof RichInput>;

export interface SearchInputProps {
  size?: React.ComponentProps<typeof RichInput>['size'];

  value: string;
  parsed: ISearchTerm[];
  onChange(value: string): void;

  inputRef?: React.RefObject<HTMLElement>;
  onActive?(active: boolean): void;
  onKeyDown?(event: React.KeyboardEvent<HTMLInputElement>): void;
}

export const SearchInput = observer(function SearchInput(props: SearchInputProps) {
  const {
    size,
    value,
    parsed,
    onChange,
    inputRef,
    onActive = noop,
    onKeyDown = noop,
  } = props;

  const controller = useSearchInputController();
  // ---
  controller.value = value;
  controller.onChange = onChange;
  controller.onActive = onActive;
  controller.onKeyDown = onKeyDown;
  // ---

  const richInputRef = React.useRef<HTMLDivElement>(null);
  const cursorAtElementRef = React.useRef<HTMLSpanElement>(null);
  const [cursorAt, setCursorAt] = React.useState(-1);

  const placeholder = useL10n('#ServerList_SearchHint2');

  const handleSelect: RichInputProps['onSelect'] = React.useCallback((start, end) => {
    setCursorAt(start === end
      ? start
      : -1);
  }, []);

  const wizardPosition = useWizardPosition(cursorAt, cursorAtElementRef, richInputRef);

  const [rendered, activeTermIndex] = React.useMemo(
    () => renderSearchInput(value, parsed, cursorAt, cursorAtElementRef),
    [value, cursorAt],
  );

  React.useEffect(() => {
    richInputRef.current?.focus();
  }, []);

  const deferredActiveTermIndex = React.useDeferredValue(activeTermIndex);
  React.useEffect(() => {
    controller.setActiveTermIndex(parsed, deferredActiveTermIndex);
  }, [controller, parsed, deferredActiveTermIndex]);

  const richInputClassName = clsx(s.root, ui.cls.fullWidth, {
    [s.focus]: controller.inputInFocus,
    [s.shadowy]: value[0] === '>',
  });

  return (
    <>
      <RichInput
        autoFocus
        withClearButton
        ref={mergeRefs(richInputRef, inputRef)}
        size={size}
        value={value}
        onBlur={controller.handleInputBlur}
        onFocus={controller.handleInputFocus}
        onKeyDown={controller.handleInputKeyDown}
        onChange={onChange}
        onSelect={handleSelect}
        rendered={rendered}
        className={richInputClassName}
        placeholder={placeholder}
      />

      <Cheatsheet controller={controller} inputRef={richInputRef} />

      <Wizard controller={controller} position={wizardPosition} />
    </>
  );
});

type WizardProps = {
  controller: SearchInputController;
  position: ReturnType<typeof useWizardPosition>;
};
const Wizard = observer(function Wizard(props: WizardProps) {
  const {
    controller,
    position,
  } = props;

  const TitleOutlet = useOutlet(TITLE_OUTLET_ID);

  if (!controller.shouldRenderWizard || !position) {
    return null;
  }

  const nodes = controller.suggestions === SuggestionState.INDEX_NOT_LOADED
    ? (
      <div className={s.loader}>
        <Indicator />
      </div>
      )
    : (
        (controller.suggestions as string[]).map((suggestion, index) => (
          <div
            key={suggestion}
            className={clsx(s.item, { [s.active]: index === controller.selectedSuggestionIndex })}
          >
            {suggestion}
          </div>
        ))
      );

  return (
    <TitleOutlet>
      <div
        className={s.wizard}
        style={
          {
            '--x': `${position.cursorX}px`,
            '--y': `${position.cursorY}px`,
          } as any
        }
      >
        <div className={s.content}>{nodes}</div>
      </div>
    </TitleOutlet>
  );
});

interface WizardPos {
  cursorX: number;
  cursorY: number;

  inputX: number;
  inputY: number;
  inputW: number;
  inputH: number;
}

function useWizardPosition(
  cursorAt: number,
  cursorAtElementRef: React.RefObject<HTMLSpanElement>,
  inputRef: React.RefObject<HTMLDivElement>,
): null | WizardPos {
  const [at, setAt] = React.useState<null | WizardPos>(null);

  const lastCursorAtElementRef = React.useRef<HTMLSpanElement | null>(null);

  const recalculatePosition = React.useCallback(() => {
    if (!cursorAtElementRef.current) {
      setAt(null);

      return;
    }

    const cursorAtRect = cursorAtElementRef.current.getBoundingClientRect();
    const richInputRect = inputRef.current?.getBoundingClientRect();

    const cursorY = richInputRect
      ? richInputRect.bottom
      : cursorAtRect.bottom;

    setAt({
      cursorX: cursorAtRect.right,
      cursorY,
      inputX: richInputRect?.x || 0,
      inputY: richInputRect?.y || 0,
      inputW: richInputRect?.width || 0,
      inputH: richInputRect?.height || 0,
    });
  }, []);

  useWindowResize(recalculatePosition);

  React.useEffect(() => {
    if (lastCursorAtElementRef.current === cursorAtElementRef.current) {
      return;
    }
    lastCursorAtElementRef.current = cursorAtElementRef.current;

    recalculatePosition();
  }, [cursorAt]);

  return at;
}

type Part = { term: ISearchTerm; index: number };

function renderSearchInput(
  value: string,
  parsed: ISearchTerm[],
  cursorAt: number,
  cursorAtElementRef: React.RefObject<HTMLSpanElement>,
): [React.ReactNode[], number] {
  let idx = 0;
  const indices = Array<number>(parsed.length * 2);
  const parts: Record<number, Part> = Object.create(null);

  for (let index = 0; index < parsed.length; index++) {
    const term = parsed[index];

    indices[idx++] = term.offset;
    indices[idx++] = term.offset + term.source.length;

    parts[term.offset] = {
      term,
      index,
    };
  }

  idx = 0;
  const split = splitByIndices(value, indices);
  const nodes = Array<React.ReactNode>(split.size);

  let activeTermIndex = -1;

  for (const [index, str] of split) {
    const key = `${index}${str}`;
    const part: undefined | Part = parts[index] as any; // as otherwise typescript will omit the `undefined` variant

    const cursorWithin = activeTermIndex === -1 && cursorAt >= index && cursorAt <= index + str.length;

    if (cursorWithin && part) {
      activeTermIndex = part.index;
    }

    const cls = clsx(s.part, part && [s[part.term.type]], {
      [s.invert]: part?.term.invert,
      [s.active]: cursorWithin,
      [s.ignored]: !part,
      [s.addressPlaceholder]: part?.term.type === 'address' && value === '>',
    });

    nodes[idx++] = (
      <span
        key={key}
        ref={part && cursorWithin
          ? cursorAtElementRef
          : undefined}
        className={cls}
      >
        {str}
      </span>
    );
  }

  return [nodes, activeTermIndex];
}
