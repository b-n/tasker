import { useState, useEffect } from 'react';
import { scaleTime, scaleOrdinal, scaleBand } from 'd3-scale';

const useTimeScale = (props) => {
  const { from, to, width } = props;
  const [scale, setScale] = useState({ scale: scaleTime().domain([from, to]) });

  useEffect(() => {
    setScale({
      scale: scale.scale
        .range([0, width])
    })
  }, [width]);

  return scale.scale;
}

const useOrdinalScale = (props) => {
  const { values, range } = props;
  const [scale] = useState({
    scale: scaleOrdinal()
      .domain(values)
      .range(range)
  });
  
  return scale.scale;
}

const useBandScale = (props) => {
  const { values, range } = props;
  const [scale] = useState({
    scale: scaleBand()
      .domain(values)
      .range(range)
      .round(true)
      .padding(0.3)
  });

  return scale.scale;
}

export {
  useTimeScale,
  useOrdinalScale,
  useBandScale,
}
