import { useInstance, noop, replaceRange } from '@cfx-dev/ui-components';
import { inject, injectable } from 'inversify';
import { makeAutoObservable, observable } from 'mobx';
import React from 'react';

import { ICategorySearchTerm, ISearchTerm, searchTermToString } from 'cfx/base/searchTermsParser';
import { useServiceResolver } from 'cfx/base/servicesContainer';
import { scopedLogger, ScopedLogger } from 'cfx/common/services/log/scopedLogger';
import { IServersService } from 'cfx/common/services/servers/servers.service';
import { clone } from 'cfx/utils/object';

export enum SuggestionState {
  NOT_AVAILABLE,
  INDEX_NOT_LOADED,
}

export function useSearchInputController() {
  return useInstance(initSearchInputContoller, useServiceResolver());
}
function initSearchInputContoller(serviceResolver: ReturnType<typeof useServiceResolver>): SearchInputController {
  return serviceResolver(SearchInputController);
}

@injectable()
export class SearchInputController {
  @scopedLogger('SearchInputController')
  protected readonly logService: ScopedLogger;

  @inject(IServersService)
  protected readonly serversService: IServersService;

  private _inputInFocus: boolean = false;
  public get inputInFocus(): boolean {
    return this._inputInFocus;
  }
  private set inputInFocus(inputInFocus: boolean) {
    this._inputInFocus = inputInFocus;
    this.onActive(inputInFocus);
  }

  private _activeTermIndex: number = -1;
  private get activeTermIndex(): number {
    return this._activeTermIndex;
  }
  private set activeTermIndex(activeTermIndex: number) {
    this._activeTermIndex = activeTermIndex;
  }

  private _selectedSuggestionIndex: number = 0;
  public get selectedSuggestionIndex(): number {
    return this._selectedSuggestionIndex;
  }
  private set selectedSuggestionIndex(selectedSuggestionIndex: number) {
    this._selectedSuggestionIndex = selectedSuggestionIndex;
  }

  onActive: (active: boolean) => void = noop;

  onKeyDown: (event: React.KeyboardEvent<HTMLInputElement>) => void = noop;

  value: string = '';

  onChange: (value: string) => void = noop;

  handleChange: (value: string) => void = noop;

  private _parsed: ISearchTerm[] = [];
  private get parsed(): ISearchTerm[] {
    return this._parsed;
  }
  private set parsed(parsed: ISearchTerm[]) {
    this._parsed = parsed;
  }

  get activeTerm(): ICategorySearchTerm | null {
    if (this.activeTermIndex === -1) {
      return null;
    }

    if (this.activeTermIndex >= this.parsed.length) {
      return null;
    }

    const term = this.parsed[this.activeTermIndex];

    if (!term) {
      return null;
    }

    if (term.type !== 'category') {
      return null;
    }

    if (term.regexp) {
      return null;
    }

    if (term.category !== 'tag' && term.category !== 'locale') {
      return null;
    }

    return term;
  }

  get suggestions(): string[] | SuggestionState {
    const {
      activeTerm,
    } = this;

    if (!activeTerm) {
      return SuggestionState.NOT_AVAILABLE;
    }

    if (activeTerm.category !== 'tag' && activeTerm.category !== 'locale') {
      return SuggestionState.NOT_AVAILABLE;
    }

    // Still loading
    if (!this.serversService.autocompleteIndex) {
      return SuggestionState.INDEX_NOT_LOADED;
    }

    const index = activeTerm.category === 'tag'
      ? this.serversService.autocompleteIndex.tag.sequence
      : this.serversService.autocompleteIndex.locale.sequence;

    const lcterm = activeTerm.value.toLowerCase();

    return index.filter((x) => x.toLowerCase().includes(lcterm)).slice(0, 10);
  }

  get shouldRenderWizard(): boolean {
    if (!this.inputInFocus) {
      return false;
    }

    const {
      activeTerm,
    } = this;

    if (!activeTerm) {
      return false;
    }

    if (activeTerm.category !== 'tag' && activeTerm.category !== 'locale') {
      return false;
    }

    const {
      suggestions,
    } = this;

    if (suggestions === SuggestionState.NOT_AVAILABLE) {
      return false;
    }

    if (suggestions === SuggestionState.INDEX_NOT_LOADED) {
      return true;
    }

    if (suggestions.length === 0) {
      return false;
    }

    if (suggestions.length === 1) {
      return suggestions[0].toLowerCase() !== activeTerm.value.toLowerCase();
    }

    return true;
  }

  readonly handleInputFocus = () => {
    this.inputInFocus = true;
  };

  readonly handleInputBlur = () => {
    this.inputInFocus = false;
  };

  constructor() {
    makeAutoObservable(this, {
      value: false,

      onChange: false,
      handleChange: false,
      handleInputKeyDown: false,
      handleInputFocus: false,
      handleInputBlur: false,

      // @ts-expect-error private
      _suggestions: observable.ref,
    });
  }

  readonly handleInputKeyDown = (event: React.KeyboardEvent<HTMLInputElement>) => {
    this.onKeyDown(event);

    if (!this.shouldRenderWizard) {
      return;
    }

    const isArrowDown = event.code === 'ArrowDown';
    const isArrowUp = event.code === 'ArrowUp';
    const isEnter = event.code === 'Enter';

    if (!isArrowDown && !isArrowUp && !isEnter) {
      return;
    }
    event.preventDefault();

    const suggestions = this.suggestions as string[];
    const suggestionIndex = this.selectedSuggestionIndex;

    if (isEnter) {
      let term = this.activeTerm;

      if (!term) {
        return;
      }

      const suggestion = suggestions[suggestionIndex];

      if (!suggestion) {
        return;
      }

      term = clone(term);
      term.value = suggestion;

      const replacement = this.activeTermIndex === this.parsed.length - 1
        ? `${searchTermToString(term)} `
        : searchTermToString(term);

      this.onChange(replaceRange(
        this.value,
        replacement,
        term.offset,
        term.offset + term.source.length,
      ));

      return;
    }

    let newSuggestionIndex = this.selectedSuggestionIndex + (
      isArrowUp
        ? -1
        : 1
    );

    if (newSuggestionIndex < 0) {
      newSuggestionIndex = suggestions.length - 1;
    } else if (newSuggestionIndex === suggestions.length) {
      newSuggestionIndex = 0;
    }

    this.selectedSuggestionIndex = newSuggestionIndex;
  };

  setActiveTermIndex(parsed: ISearchTerm[], termIndex: number) {
    this.parsed = parsed;
    this.activeTermIndex = termIndex;
    this.selectedSuggestionIndex = 0;
  }
}
