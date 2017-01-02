1. Fetch available versions for each dependency
2. If a project is already in the graph, intersect the new requirement with the one already present
3. Select the highest [precedence] version which satisfies the intersected requirement (or only requirement, if this is the first insertion)
  * This always has to be O(n) over the list of versions, because custom requirements mean that a previously failing version might become a candidate again
4. Collect all child dependencies from that graph depth, repeat from step 1

### In event of a conflict

Given a project already in the graph with requirement X, and about to be re-added to the graph with requirement Y, where X and Y conflict:

1. Find the _dependent_ project which changed the requirement into X (either from nil, or from a previously looser requirement)
2. Select the next best version of that dependent project and attempt to finish resolving
3. In the event that no other versions satisfy the requirements applied to the dependent, we have two choices:
  - GOTO step 1 for the dependent project, or
  - Find a further back time where the originally conflicting dependency (the one with requirement X) was added (e.g., with requirement W), and try replacing the dependent which added it at _that_ level instead

### Open questions

- With regard to conflict resolution, which version of step 3 is better?
  - Is it better to minimize churn in the graph, or to prefer a more holistic/less local view of resolving the graph?
- How does this perform on very deep graphs, like npm projects often have?
  - It may be better because it doesn't perform an exhaustive search like we do now
  - However, this algorithm may result in _over-backtracking_ if conflicts are common
- Do the benefits of this approach justify the increased complexity?
  - This algorithm is more work to maintain and understand
  - Would a user be able to understand what went wrong if version resolution fails?
- Does this introduce more ways that sibling projects could interfere with each other?
  - If projects A and B are battling each other over a shared dependency C, A could end up downgrading B by many versions
  - This is less likely with the pseudo-breadth-first search that we have now, because specific projects shouldn't be more affected than any others
  - Could we implement some sort of "fairness" calculation that minimizes this possibility?
