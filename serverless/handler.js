'use strict';

const AWS = require('aws-sdk');

module.exports.iot = async (event, context) => {
  AWS.config.update({region: process.env.AWS_REGION});

  const dynamodb = new AWS.DynamoDB.DocumentClient({apiVersion: '2012-08-10'});
  const dynamoTable = process.env.DYNAMODB_TABLE;
  
  const { thing, newProject, oldProject, timestamp } = event;

  const result = await dynamodb.query({
    TableName: dynamoTable,
    KeyConditions: {
      'id': {
        ComparisonOperator: 'EQ',
        AttributeValueList: [ thing ]
      }
    }
  }).promise()

  const record = result.Count > 0
    ? result.Items[0]
    : { id: thing, rawHistory: [], projectHistory: [], currentProjects: [], projects: [], allProjects: []};

  //break project number into actual array
  const projects = new Array(8).fill(0).reduce((p, c, i) => {
    newProject & (1 << i) ? p.push(i) : null;
    return p;
  }, []);

  const newHistoryRecords = record.currentProjects
    .filter(p => projects.indexOf(p.project) === -1)
    .map(p => {
      const { project, from } = p;
      const projectName = record.projects[project] || project;
      return { project, projectName, from, to: timestamp };
    });

  const currentProjects = record.currentProjects
    .map(p => p.project);

  const newCurrentProjects = projects.reduce((p, c) => {
    if (currentProjects.indexOf(c) === -1)
      p.push({project: c, from: timestamp});
    return p;
  },[]);

  record.currentProjects = record.currentProjects
    .concat(newCurrentProjects)
    .filter(p => projects.indexOf(p.project) !== -1);

  record.projectHistory = record.projectHistory.concat(newHistoryRecords);
  record.rawHistory.push({timestamp, newProject, oldProject});

  await dynamodb.put({
    TableName: dynamoTable,
    Item: record
  }).promise();

  context.succeed('Success');

};

module.exports.thing = async (event, context) => {
  AWS.config.update({region: process.env.AWS_REGION});

  const dynamodb = new AWS.DynamoDB.DocumentClient({apiVersion: '2012-08-10'});
  const dynamoTable = process.env.DYNAMODB_TABLE;
  
  const { thing } = event.pathParameters;
  try {
    const result = await dynamodb.query({
      TableName: dynamoTable,
      KeyConditions: {
        'id': {
          ComparisonOperator: 'EQ',
          AttributeValueList: [ thing ]
        }
      }
    }).promise()
    const thingResult = result.Items[0];
    const projectItems = thingResult.projectHistory.map(p => ({
      project: p.project,
      from: new Date(0).setUTCSeconds(p.from),
      to: new Date(0).setUTCSeconds(p.to),
      projectName: p.projectName
    }))

    const response = {
      thing,
      allProjects: thingResult.allProjects,
      projectItems
    }
    return {
      body: JSON.stringify(response),
      headers: {
        "Access-Control-Allow-Origin" : "*",
        "Access-Control-Allow-Credentials" : true
      },
      statusCode: 200
    }
  } catch (e) { 
    console.log(e);
    context.fail(e);
  }

}
