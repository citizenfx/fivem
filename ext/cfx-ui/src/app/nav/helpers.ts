export type NavConfig = {
	withBackground: boolean,
	withHomeButton: boolean,
};

export function getNavConfigFromUrl(url: string): NavConfig {
	const isHome = url.startsWith('/home') || url === '/';

	return {
		withBackground: !isHome,
		withHomeButton: !isHome,
	};
}
