import { observer } from "mobx-react-lite";
import { useIntlService } from "./intl.service";
import { PluralKeys } from "./types";

export interface LocalizeProps {
  children?: undefined | void,

  id: string,
  args?: Record<string, any>,
}
export const Localize = observer(function Localize(props: LocalizeProps) {
  return useL10n(props.id, props.args) as any;
});

export interface LocalizePluralProps {
  children?: undefined | void,

  ids: PluralKeys,
  count: number,
  args?: Record<string, any>,
}
export const LocalizePlural = observer(function LocalizePlural(props: LocalizePluralProps) {
  return useL10nPlural(props.count, props.ids, props.args) as any;
});

export function $L(key: string, args?: Record<string, any>) {
  return <Localize id={key} args={args} />;
}
export function useL10n(key: string, args?: Record<string, any>) {
  return useIntlService().translate(key, args);
}

export function $LP(count: number, keys: PluralKeys, args?: Record<string, unknown>) {
  return <LocalizePlural count={count} ids={keys} args={args} />;
}
export function useL10nPlural(count: number, keys: PluralKeys, args?: Record<string, unknown>): string {
  return useIntlService().translatePlural(count, keys, args);
}
