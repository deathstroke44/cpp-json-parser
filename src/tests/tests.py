import os
import time
start_time = time.time()
os.chdir('..')
file = open('output.txt','w')
# file.write("Output of Json path query script:\n\n")
file.close()
relative_path = 'tests/Json files/'
tests = [
    {
        'fileName': 'more-case-3.json',
        'query': '$.[0]'
    },
    {
        'fileName': 'more-case-2.json',
        'query': '$.phoneNumbers[*].type'
    },
    {
        'fileName': 'more-case-2.json',
        'query': '$.phoneNumbers..type'
    },
    {
        'fileName': 'more-case.json',
        'query': '$..*'
    },
    {
        'fileName': 'more-case.json',
        'query': '$..user'
    },{
        'fileName': 'more-case.json',
        'query': '$..*'
    },
    {
        'fileName': 'more-case.json',
        'query': '$..name'
    },
    {
        'fileName': 'code-test.json',
        'query': '$.skills'
    },
    {
        'fileName': 'code-test.json',
        'query': '$.skills.lang'
    },
    {
        'fileName': 'code-test.json',
        'query': '$.skills.lang[*]'
    },
    {
        'fileName': 'code-test.json',
        'query': '$.skills.lang[0]'
    },
    {
        'fileName': 'code-test.json',
        'query': '$.skills.lang[1]'
    },
    {
        'fileName': 'code-test.json',
        'query': '$.skills.lang[1][0]'
    },
    {
        'fileName': 'code-test.json',
        'query': '$.skills.lang[2]'
    },
    {
        'fileName': 'code-test.json',
        'query': '$.skills.lang[0].C'
    },
    {
        'fileName': 'code-test.json',
        'query': '$.skills.lang[0].C[1]'
    },
    {
        'fileName': 'code-test.json',
        'query': '$.skills.nested1.nested3'
    },
    {
        'fileName': 'code-test.json',
        'query': '$.skills.nested1.nested3.key2'
    },
    {
        'fileName': 'code-test.json',
        'query': '$.skills.test_array'
    },
    {
        'fileName': 'code-test.json',
        'query': '$.skills.test_array[2]'
    },
    {
        'fileName': 'code-test.json',
        'query': '$.skills.test_array[2][2]'
    },
    {
        'fileName': 'code-test.json',
        'query': '$.skills.test_array[2][2][2]'
    },
    {
        'fileName': 'code-test.json',
        'query': '$.skills.test_array[2][2][2][0]'
    },
    {
        'fileName': 'code-test.json',
        'query': '$.skills.test_array[2][2][2][0][0]'
    },
    {
        'fileName': 'code-test.json',
        'query': '$.skills.test_array[2][2][2][0][1]'
    },
    {
        'fileName': 'large-file.json',
        'query': '$.[0].actor'
    },
    {
        'fileName': 'large-file.json',
        'query': '$.[0].repo'
    },
    {
        'fileName': 'large-file.json',
        'query': '$.[0].repo.url'
    },
    {
        'fileName': 'Sample-employee-JSON-data.json',
        'query': '$.Employees[0]'
    },
    {
        'fileName': 'Sample-employee-JSON-data.json',
        'query': '$.Employees[*]'
    },
    {
        'fileName': 'Sample-employee-JSON-data.json',
        'query': '$.Employees[0].jobTitle'
    },
    {
        'fileName': 'Sample-employee-JSON-data.json',
        'query': '$.Employees[*].jobTitle'
    },
    {
        'fileName': 'Sample-employee-JSON-data.json',
        'query': '$..[*].jobTitle'
    },
    {
        'fileName': 'Sample-employee-JSON-data.json',
        'query': '$.Employees[*].*'
    },
    {
        'fileName': 'store.json',
        'query': '$.store.book[*].author'
    },
    {
        'fileName': 'store.json',
        'query': '$.store.*'
    }
]
os.system('clear')
os.system('g++ jsonPathQueryNew.cpp')
for test in tests:
    sys_call= './a.out '+ test['fileName']+' '+ test['query']
    os.system(sys_call)
print('Execution time:', time.time()-start_time);