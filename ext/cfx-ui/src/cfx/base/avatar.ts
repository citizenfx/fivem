export function getAvatarURL(template: string, size = 250): string {
  const prefix = template[0] === '/'
    ? 'https://forum.cfx.re'
    : '';

  return prefix + template.replace('{size}', size.toString());
}
