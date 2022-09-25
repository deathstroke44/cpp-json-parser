import os
os.chdir('..')
file = open('output.txt','w')
# file.write("Output of Json path query script:\n\n")
file.close()
relative_path = 'tests/Json files/'
tests = [
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
        'query': '$.[0]'
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
os.system('g++ json-path-query-demo.cpp')
for test in tests:
    sys_call= './a.out '+ test['fileName']+' '+ test['query']
    os.system(sys_call)