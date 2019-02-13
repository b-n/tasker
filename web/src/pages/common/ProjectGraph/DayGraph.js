import React from 'react'
import { Collection } from '@potion/layout'
import { Group, Text, Rect } from '@potion/element'
import { addHours, format } from 'date-fns'

import { XAxis, YAxis } from './Axes'

import './index.css'

import { useTimeScale, useBandScale } from '../../../hooks/graphScales'

const DayGraph = (props) => {
  const {
    x,
    y,
    width,
    date,
    data,
    projectScale
  } = props;

  const from = addHours(date, 7),
        to = addHours(date, 18);

  const xScale = useTimeScale({ width, from, to });
  const yScale = useBandScale({ values: projectScale.domain(), range: [0, 100]})
  const bandWidth = yScale.bandwidth();

  const keyedData = data.map((d,i) => ({key: '' + i, ...d}));

  return (
    <Group transform={{translate: [x, y]}}>
      <Text y={-2} x={5}>{format(date, 'ddd\tDD\tMMM')}</Text>
      <XAxis
        scale={xScale}
        transform={{translate: [0, 100]}}
        gridline={60}
        className="x-axis"
      />
      <YAxis
        scale={yScale}
        gridline={width}
        className="y-axis"
      />
      <Collection
        data={keyedData}
        nodeEnter={d => ({ ...d, to: d.from })}
        animate
      >
        {nodes => nodes.map(({key, from, to, projectName}) => {
          const x = xScale(new Date(from));
          const width = xScale(new Date(to)) - x;
          const y = yScale(projectName);
          return <Rect key={key} x={x} y={y-bandWidth/2} width={width} height={bandWidth} fill={projectScale(projectName)}/>
        })}
      </Collection>
    </Group>
  );
}

export default DayGraph;
