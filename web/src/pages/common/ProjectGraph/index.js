import React from 'react';
import { Svg } from '@potion/element'
import { schemeCategory10 } from 'd3-scale-chromatic';

import DayGraph from './DayGraph'
import { useOrdinalScale } from '../../../hooks/graphScales'

const ProjectGraph = (props) => {
  const { data, width } = props;
  const { allProjects, projectData } = data;

  const projectScale = useOrdinalScale({values: allProjects, range: schemeCategory10 });

  const margin = { top: 30, bottom: 0, left: 130, right: 30 }

  const height = margin.top + margin.bottom + projectData.length*160;

  return (
    <Svg height={height} width={width}>
      {projectData.length > 0 ? projectData.map((d, i) =>
        <DayGraph
          x={margin.left}
          y={i*160 + margin.top}
          width ={width - margin.left - margin.right}
          key={d.date}
          date={d.date}
          data={d.data} 
          projectScale={projectScale}
        />
      ) : null}
    </Svg>
  )
}

export default ProjectGraph;
