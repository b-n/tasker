import React from 'react';
import useProjectData from '../../hooks/projectData'
import ProjectGraph from '../common/ProjectGraph'
import { useWindowWidth } from '../../hooks/window'

const Home = (props) => {
  const projectData = useProjectData('tasker');  

  const width = useWindowWidth();

  return (
    <div>
      <section>
        {projectData !== null
          ?  <ProjectGraph data={projectData} width={width} />
          : null
        }
      </section>
    </div>
  )
}

export default Home;

