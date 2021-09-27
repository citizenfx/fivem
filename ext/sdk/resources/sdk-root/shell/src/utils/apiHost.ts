const port = parseInt(location.port, 10) || 80;
const host = location.hostname;

export const apiHost = {
  port,
  host,
};
