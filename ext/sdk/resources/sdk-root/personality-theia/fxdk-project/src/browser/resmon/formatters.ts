import { rgbForKey } from "../utils/color";

function f(n: number): string {
  return n.toFixed(2);
}

export function formatResourceName(cell: unknown): unknown {
  if (typeof cell === 'string') {
    const [r, g, b] = rgbForKey(cell);

    return `<span class=resource-name style="background:rgb(${r}, ${g}, ${b})">${cell}</span>`;
  }

  return cell;
}

export function formatMS(cell: unknown): unknown {
  if (typeof cell === 'number') {
    if (cell <= 0.005) {
      return '—';
    }

    return cell.toFixed(2) + ' ms';
  }

  return cell;
}

export function formatTimePercentage(cell: unknown): unknown {
  if (typeof cell === 'number') {
    if (cell < 0.005) {
      return '—';
    }

    const intensityColor = `rgba(244, 5, 82, ${cell})`;

    return `<span style="padding: 0 2px;border: solid 2px ${intensityColor}">${(cell * 100).toFixed(2)}%</span>`;
  }

  return cell;
}

export function formatMemory(postfix: string, mem: unknown): unknown {
  if (typeof mem === 'number') {
    if (mem <= 0) {
      return '?';
    }

    switch (true) {
      case mem >= (1024 ** 3): {
        return f(mem / (1024 ** 3)) + ' GiB' + postfix;
      }
      case mem >= (1024 ** 2): {
        return f(mem / (1024 ** 2)) + ' MiB' + postfix;
      }
      case mem >= 1024: {
        return f(mem / 1024) + ' KiB' + postfix;
      }
      default: {
        return f(mem) + ' B' + postfix;
      }
    }
  }

  return mem;
}
