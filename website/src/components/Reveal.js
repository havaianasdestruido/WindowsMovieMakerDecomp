import React, {useEffect, useRef} from 'react';

/**
 * Scroll-reveal wrapper: fade + rise once when the element enters the viewport.
 *
 * Motion discipline (see project design notes):
 * - IntersectionObserver only. No window scroll listeners, no rAF loops.
 * - Animates transform/opacity exclusively (GPU-friendly).
 * - Collapses to fully visible under prefers-reduced-motion, and when JS is
 *   unavailable the SSR markup is visible by default (no hidden-by-default
 *   flash-or-blank states).
 */
export default function Reveal({as: Tag = 'div', delay = 0, className = '', children, ...rest}) {
  const ref = useRef(null);

  useEffect(() => {
    const el = ref.current;
    if (!el) return undefined;

    if (window.matchMedia('(prefers-reduced-motion: reduce)').matches) {
      return undefined;
    }

    el.classList.add('wmmr-reveal');
    el.style.setProperty('--wmmr-reveal-delay', `${delay}ms`);

    const io = new IntersectionObserver(
      (entries) => {
        for (const entry of entries) {
          if (entry.isIntersecting) {
            el.classList.add('wmmr-reveal--visible');
            io.disconnect();
          }
        }
      },
      {threshold: 0.15, rootMargin: '0px 0px -6% 0px'},
    );
    io.observe(el);

    return () => {
      io.disconnect();
      el.classList.remove('wmmr-reveal', 'wmmr-reveal--visible');
      el.style.removeProperty('--wmmr-reveal-delay');
    };
  }, [delay]);

  return (
    <Tag ref={ref} className={className} {...rest}>
      {children}
    </Tag>
  );
}
