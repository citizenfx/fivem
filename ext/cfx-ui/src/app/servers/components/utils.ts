export function getCanonicalLocale(locale: string): string {
    try {
        return (Intl as any).getCanonicalLocales(locale.replace(/_/g, '-'))[0];
    } catch {
        return 'root-AQ';
    }
}
